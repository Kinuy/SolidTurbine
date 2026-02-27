/**
 * @file NingSolver.cpp
 * @brief Implementation of the Ning (2013) BEM solver.
 *
 * ── Access model ─────────────────────────────────────────────────────────────
 *
 *  Blade geometry and polars   →  cfg_.turbine    (TurbineGeometry*)
 *  Physics constants, numerics →  cfg_.sim_config (ISimulationConfig*)
 *  Flow velocities             →  cfg_.flow_calculator (FlowCalculator*)
 *
 * All numerical magic lives here.  Aerodynamic models (loss, induction,
 * root-finding) are called through injected interfaces so this file never
 * needs to change when models change (Open/Closed).
 */
#define _USE_MATH_DEFINES
#include "NingSolver.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

#include "FlowCalculator.h"
#include "TurbineGeometry.h"
#include "ISimulationConfig.h"

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────
NingSolver::NingSolver(SolverConfig cfg)
    : cfg_(std::move(cfg)), num_sections_(cfg_.turbine->num_sections())
{
    if (!cfg_.loss_model || !cfg_.induction_model || !cfg_.root_finder)
        throw std::invalid_argument("NingSolver: all model dependencies must be non-null");
    if (!cfg_.turbine || !cfg_.sim_config || !cfg_.flow_calculator)
        throw std::invalid_argument("NingSolver: turbine, sim_config and flow_calculator must be non-null");

    result_.phi.assign(num_sections_, 0.0);
    result_.a_ind_axi.assign(num_sections_, 0.0);
    result_.a_ind_rot.assign(num_sections_, 0.0);
    result_.v_inf = cfg_.flow_calculator->v_inf();
    result_.lambda = cfg_.flow_calculator->Lambda();
    result_.pitch = cfg_.pitch;
    result_.psi = cfg_.psi;

    sec_solidity_.resize(num_sections_);
    beta_.resize(num_sections_);
    k_cache_.resize(num_sections_, 0.0);
    converged_.assign(num_sections_, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Public entry point
// ─────────────────────────────────────────────────────────────────────────────
bool NingSolver::Solve()
{
    Initialise();
    SolveAllSections();
    FinaliseResult();
    return result_.success;
}

// ─────────────────────────────────────────────────────────────────────────────
// Initialise per-section constants
// ─────────────────────────────────────────────────────────────────────────────
void NingSolver::Initialise()
{
    if (cfg_.verbose)
        LogOperatingPoint();

    [[maybe_unused]] double v_tip = cfg_.flow_calculator->Lambda() * cfg_.flow_calculator->v_inf();

    for (std::size_t i = 0; i < num_sections_; ++i)
    {
        // Solidity: sigma = B * c / (2*pi*r)
        sec_solidity_[i] = cfg_.turbine->num_blades_as_double() * cfg_.turbine->chord(i) / (2.0 * M_PI * cfg_.turbine->radius(i));

        // Effective pitch: twist + collective pitch + aeroelastic torsion
        beta_[i] = cfg_.turbine->twist(i) + cfg_.pitch; // + cfg_.turbine->TorsionAtR(i, v_tip, cfg_.flow_calculator->v_inf());

        result_.a_ind_axi[i] = 0.3;
        result_.a_ind_rot[i] = 0.0;
        result_.phi[i] = 0.0;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Solve all blade sections
// ─────────────────────────────────────────────────────────────────────────────
void NingSolver::SolveAllSections()
{
    for (std::size_t sec = 0; sec < num_sections_; ++sec)
    {
        if (!FindSolutionPositiveRegion(sec))
            FindSolutionNegativeRegion(sec);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Determine overall success
// ─────────────────────────────────────────────────────────────────────────────
void NingSolver::FinaliseResult()
{
    result_.success = std::all_of(converged_.begin(), converged_.end(),
                                  [](int c)
                                  { return c == 1; });
    if (!result_.success)
        LogFailedSections();
}

// ─────────────────────────────────────────────────────────────────────────────
// Search 0 to pi for a valid root
// ─────────────────────────────────────────────────────────────────────────────
bool NingSolver::FindSolutionPositiveRegion(std::size_t sec)
{
    constexpr double eps = 1e-6;
    const double half_pi = M_PI / 2.0;

    for (auto [lo, hi] : {std::pair{eps, half_pi},
                          std::pair{half_pi, M_PI - eps}})
    {
        if (Residual(lo, sec) * Residual(hi, sec) < 0.0)
        {
            auto root = cfg_.root_finder->Solve(
                [&](double phi)
                { return Residual(phi, sec); }, lo, hi);
            if (root && k_cache_[sec] > -1.0)
            {
                result_.phi[sec] = *root;
                converged_[sec] = 1;
                return true;
            }
            converged_[sec] = 0;
        }
    }
    for (auto [lo, hi] : FindBrackets(eps, M_PI - eps, sec))
    {
        auto root = cfg_.root_finder->Solve(
            [&](double phi)
            { return Residual(phi, sec); }, lo, hi);
        if (root && k_cache_[sec] > -1.0)
        {
            result_.phi[sec] = *root;
            converged_[sec] = 1;
            return true;
        }
        converged_[sec] = 0;
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Search 0 to -pi (propeller-brake region)
// ─────────────────────────────────────────────────────────────────────────────
bool NingSolver::FindSolutionNegativeRegion(std::size_t sec)
{
    constexpr double eps = 1e-6;
    const double half_pi = M_PI / 2.0;

    for (auto [lo, hi] : {std::pair{-half_pi, -eps},
                          std::pair{-M_PI + eps, -half_pi}})
    {
        if (Residual(lo, sec) * Residual(hi, sec) < 0.0)
        {
            auto root = cfg_.root_finder->Solve(
                [&](double phi)
                { return Residual(phi, sec); }, lo, hi);
            if (root && k_cache_[sec] > 1.0)
            {
                result_.phi[sec] = *root;
                converged_[sec] = 1;
                return true;
            }
            converged_[sec] = 0;
        }
    }
    for (auto [lo, hi] : FindBrackets(-M_PI + eps, -eps, sec))
    {
        auto root = cfg_.root_finder->Solve(
            [&](double phi)
            { return Residual(phi, sec); }, lo, hi);
        if (root && k_cache_[sec] > 1.0)
        {
            result_.phi[sec] = *root;
            converged_[sec] = 1;
            return true;
        }
        converged_[sec] = 0;
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// zbrak: find sign-change brackets on [x1, x2]
// ─────────────────────────────────────────────────────────────────────────────
std::vector<std::pair<double, double>>
NingSolver::FindBrackets(double x1, double x2, std::size_t sec) const
{
    constexpr unsigned n = 80;
    std::vector<std::pair<double, double>> brackets;
    double dx = (x2 - x1) / n;
    double x = x1;
    auto &self = const_cast<NingSolver &>(*this);
    double fp = self.Residual(x1, sec);
    for (unsigned i = 0; i < n; ++i)
    {
        x += dx;
        double fc = self.Residual(x, sec);
        if (fc * fp <= 0.0)
            brackets.emplace_back(x - dx, x);
        fp = fc;
    }
    return brackets;
}

// ─────────────────────────────────────────────────────────────────────────────
// Residual  f(phi) = sin(phi)/(1-a) - cos(phi)/(lambda_loc * (1-k_rot))
// ─────────────────────────────────────────────────────────────────────────────
double NingSolver::Residual(double phi, std::size_t sec)
{
    result_.phi[sec] = phi;
    result_.a_ind_axi[sec] = 0.0;
    result_.a_ind_rot[sec] = 0.0;

    double k{0}, k_rot{0};
    for (int re_it = 0; re_it < 2; ++re_it)
        EvaluatePolarAndInduction(phi, sec, k, k_rot);

    k_cache_[sec] = k;

    double local_lambda = cfg_.flow_calculator->LocalLambda(sec);
    if (phi > 0.0)
    {
        return std::sin(phi) / (1.0 - result_.a_ind_axi[sec]) - std::cos(phi) / local_lambda * (1.0 - k_rot);
    }
    else
    {
        return std::sin(phi) * (1.0 - k) - std::cos(phi) / local_lambda * (1.0 - k_rot);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Evaluate polar, loss factor and induction for one residual call
// ─────────────────────────────────────────────────────────────────────────────
void NingSolver::EvaluatePolarAndInduction(double phi,
                                           std::size_t sec,
                                           double &k_out,
                                           double &k_rot_out)
{
    double alpha = phi - beta_[sec];

    // Polar lookup: TurbineGeometry owns the blade sections and their polars.
    double Cl{0}, Cd{0}, Cm{0};
    cfg_.turbine->InterpForCoeff(sec,
                                 LocalReynoldsNumber(sec),
                                 LocalMachNumber(sec),
                                 alpha, &Cl, &Cd, &Cm);

    // Per Ning (2013): drag excluded from residual to guarantee convergence.
    Cd = 0.0;

    // Loss model input — geometry from turbine, tip_extra from sim_config
    LossModelInput li;
    li.radius = cfg_.turbine->radius(sec);
    li.rotor_radius = cfg_.turbine->RotorRadius();
    li.hub_radius = cfg_.turbine->radius(0);
    li.phi = phi;
    li.num_blades = cfg_.turbine->num_blades_as_double();
    li.chord = cfg_.turbine->chord(cfg_.turbine->num_sections() - 1);
    li.tip_extra_distance = cfg_.sim_config->tip_extra_distance();

    double F = cfg_.loss_model->Evaluate(li);
    double cn = Cl * std::cos(phi) + Cd * std::sin(phi);
    double ct = Cl * std::sin(phi) - Cd * std::cos(phi);

    k_out = sec_solidity_[sec] * cn / (4.0 * F * std::sin(phi) * std::sin(phi));
    k_rot_out = sec_solidity_[sec] * ct / (4.0 * F * std::cos(phi) * std::sin(phi));

    InductionInput ii{k_out, k_rot_out, phi, F};
    auto factors = cfg_.induction_model->Compute(ii);
    result_.a_ind_axi[sec] = factors.a_axi;
    result_.a_ind_rot[sec] = factors.a_rot;
}

// ─────────────────────────────────────────────────────────────────────────────
// Accessors
// ─────────────────────────────────────────────────────────────────────────────
double NingSolver::v_inf() const
{
    return cfg_.flow_calculator->v_inf();
}
double NingSolver::lambda() const
{
    return cfg_.flow_calculator->Lambda();
}

double NingSolver::LocalFlowVel(std::size_t sec) const
{
    double ax{0}, tan{0};
    cfg_.flow_calculator->BladeLocalVelocities(sec, &ax, &tan);
    ax *= (1.0 - result_.a_ind_axi[sec]);
    tan *= (1.0 + result_.a_ind_rot[sec]);
    return std::sqrt(ax * ax + tan * tan);
}

double NingSolver::LocalReynoldsNumber(std::size_t sec) const
{
    return LocalFlowVel(sec) * cfg_.turbine->chord(sec) / cfg_.sim_config->kinematic_viscosity(); // nu from config
}

double NingSolver::LocalMachNumber(std::size_t sec) const
{
    return LocalFlowVel(sec) / cfg_.sim_config->speed_of_sound(); // a from config
}

// ─────────────────────────────────────────────────────────────────────────────
// Logging
// ─────────────────────────────────────────────────────────────────────────────
void NingSolver::LogOperatingPoint() const
{
    std::cout << " * Solving: pitch=" << cfg_.pitch //* 180.0 / M_PI
              << "deg  lambda=" << cfg_.flow_calculator->Lambda()
              << "  v_inf=" << cfg_.flow_calculator->v_inf()
              << "  psi=" << cfg_.psi * 180.0 / M_PI << "deg\n";
}

void NingSolver::LogFailedSections() const
{
    std::cout << " * ERROR: Solver failed to converge at blade sections:\n";
    for (std::size_t i = 0; i < num_sections_; ++i)
    {
        if (converged_[i] == 0)
            std::cout << "   Section " << (i + 1)
                      << " (r/R = "
                      << cfg_.turbine->radius(i) / cfg_.turbine->RotorRadius()
                      << ")\n";
    }
}
