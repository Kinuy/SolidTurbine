/**
 * @file OperationSolver.cpp
 * @brief Implementation of OperationSolver — the outer power-curve iteration loop.
 *
 * Replaces the monolithic opermot::solve() while keeping the same numerical
 * logic.  Physics calls are delegated to the injected ITurbineController
 * and the BEMCallback; this class owns only the convergence loop.
 */
#define _USE_MATH_DEFINES
#include "OperationSolver.h"
#include "TurbineGeometry.h"
#include "ISimulationConfig.h"
#include "VariableSpeedController.h" // needed for dynamic_cast in EtaFromController

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// OperationSolverParams::FromConfig
// ─────────────────────────────────────────────────────────────────────────────
OperationSolverParams OperationSolverParams::FromConfig(
    TurbineGeometry const *turbine,
    ISimulationConfig const *sim_config)
{
    if (!turbine)
        throw std::invalid_argument("OperationSolverParams: turbine is null");
    if (!sim_config)
        throw std::invalid_argument("OperationSolverParams: sim_config is null");

    OperationSolverParams p;
    p.air_density = sim_config->air_density(); // [kg/m³]
    p.rotor_radius = turbine->RotorRadius();   // [m]
    p.p_max = sim_config->rated_power();       // [W]
    return p;
}

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────
OperationSolver::OperationSolver(OperationSolverParams params,
                                 ITurbineController const *controller,
                                 BEMCallback bem_callback)
    : p_(params), controller_(controller), bem_(std::move(bem_callback))
{
    if (!controller_)
        throw std::invalid_argument("OperationSolver: controller must be non-null");
    if (!bem_)
        throw std::invalid_argument("OperationSolver: BEM callback must be valid");
}

// ─────────────────────────────────────────────────────────────────────────────
// Run — iterate over all wind speeds at fixed pitch_deg
// ─────────────────────────────────────────────────────────────────────────────
std::vector<PowerCurvePoint>
OperationSolver::Run(double pitch_deg, std::vector<double> const &vinf_vec)
{
    if (vinf_vec.empty())
        return {};

    std::vector<PowerCurvePoint> results;
    results.reserve(vinf_vec.size());

    // Warm-start vtip: ask the controller what it would do at the first wind
    // speed with zero electrical power (sub-rated, initial guess).
    ControllerInput ci0{vinf_vec.front(), 0.0};
    ControllerOutput co0 = controller_->ComputeOperatingPoint(ci0);
    double vtip = co0.vtip;

    for (double vinf : vinf_vec)
    {
        PowerCurvePoint pt = ConvergeOnePoint(vinf, vtip, pitch_deg);
        vtip = pt.vtip; // warm-start next wind speed
        results.push_back(pt);
    }
    return results;
}

// ─────────────────────────────────────────────────────────────────────────────
// ConvergeOnePoint — inner loop for a single wind speed
// ─────────────────────────────────────────────────────────────────────────────
PowerCurvePoint OperationSolver::ConvergeOnePoint(double vinf,
                                                  double &vtip_inout,
                                                  double pitch_deg)
{
    PowerCurvePoint pt{};
    pt.vinf = vinf;
    pt.p_wind = 0.5 * p_.air_density * std::pow(vinf, 3) * M_PI * std::pow(p_.rotor_radius, 2);

    double gamma = pitch_deg; // collective pitch [deg]
    double p_el = 0.0;
    double vtip = vtip_inout;
    double lambda = 0;

    for (int iter = 0; iter < static_cast<int>(p_.max_iter); ++iter)
    {
        // ── BEM solve ──────────────────────────────────────────────────────
        lambda = (vinf > 0.0) ? vtip / vinf : 0.0;
        auto [cp, ct] = bem_(vinf, lambda, gamma);

        // ── Mechanical quantities ──────────────────────────────────────────
        double p_aero = cp * pt.p_wind;
        double omega = (p_.rotor_radius > 0.0) ? vtip / p_.rotor_radius : 0.0;
        double torque = (omega > 1e-9) ? p_aero / omega : 0.0;
        double n_rpm = omega * 60.0 / (2.0 * M_PI);

        // ── Drivetrain efficiency (queried via controller) ─────────────────
        double eta = EtaFromController(p_aero);
        double p_el_new = p_aero * eta;

        // ── Above-rated check ──────────────────────────────────────────────
        if (p_el_new >= p_.p_max && iter >= static_cast<int>(p_.min_iter))
        {
            ControllerOutput co = controller_->ComputeOperatingPoint({vinf, p_.p_max});
            vtip = co.vtip;
            gamma = co.pitch;
            lambda = (vinf > 0.0) ? vtip / vinf : 0.0;

            auto [cp2, ct2] = bem_(vinf, lambda, gamma);
            p_aero = cp2 * pt.p_wind;
            omega = (p_.rotor_radius > 0.0) ? vtip / p_.rotor_radius : 0.0;
            torque = (omega > 1e-9) ? p_aero / omega : 0.0;
            n_rpm = omega * 60.0 / (2.0 * M_PI);
            eta = EtaFromController(p_aero);
            p_el = std::min(eta * p_aero, p_.p_max);

            FillResult(pt, vtip, lambda, gamma, cp2, p_aero, n_rpm, torque, eta, p_el, ct2);
            vtip_inout = vtip;
            return pt;
        }

        // ── Controller update ──────────────────────────────────────────────
        ControllerOutput co = controller_->ComputeOperatingPoint({vinf, p_el_new});
        double gamma_new = co.pitch;
        double vtip_new = co.vtip;

        // Damped update to avoid oscillation
        double res = std::abs(gamma_new - gamma);
        gamma = 0.5 * (gamma + gamma_new);
        vtip = 0.5 * (vtip + vtip_new);
        p_el = p_el_new;

        // ── Convergence check ──────────────────────────────────────────────
        if (res < 1e-3 && iter >= static_cast<int>(p_.min_iter))
        {
            FillResult(pt, vtip, lambda, gamma, cp, p_aero, n_rpm, torque, eta, p_el, ct);
            vtip_inout = vtip;
            return pt;
        }
    }

    std::cout << " * WARNING: OperationSolver did not converge for v_inf = "
              << vinf << " m/s\n";

    // Return best estimate even if not fully converged
    // double lambda = (vinf > 0.0) ? vtip / vinf : 0.0;
    // auto [cp, ct] = bem_(vinf, lambda, gamma);
    // double p_aero = cp * pt.p_wind;
    // double omega = (p_.rotor_radius > 0.0) ? vtip / p_.rotor_radius : 0.0;
    // double torque = (omega > 1e-9) ? p_aero / omega : 0.0;
    // double n_rpm = omega * 60.0 / (2.0 * M_PI);
    // double eta = EtaFromController(p_aero);
    // FillResult(pt, vtip, lambda, gamma, cp, p_aero, n_rpm, torque, eta, p_el, ct);
    // vtip_inout = vtip;
    return pt;
}

// ─────────────────────────────────────────────────────────────────────────────
// FillResult — populate output struct
// ─────────────────────────────────────────────────────────────────────────────
void OperationSolver::FillResult(PowerCurvePoint &pt,
                                 double vtip, double lambda, double pitch,
                                 double cp, double p_aero,
                                 double n_rpm, double torque,
                                 double eta, double p_el, double ct) const
{
    pt.vtip = vtip;
    pt.lambda = lambda;
    pt.pitch = pitch;
    pt.cp_aero = cp;
    pt.p_aero = p_aero;
    pt.n = n_rpm;
    pt.torque = torque;
    pt.eta = eta;
    pt.p_el = p_el;
    pt.ct = ct;
}

// ─────────────────────────────────────────────────────────────────────────────
// EtaFromController — query efficiency via dynamic_cast if available
// ─────────────────────────────────────────────────────────────────────────────
double OperationSolver::EtaFromController(double p_aero) const
{
    // Try to query the concrete controller for its efficiency model.
    // If the controller does not expose Eta(), fall back to 0.85.
    if (auto *vsc = dynamic_cast<VariableSpeedController const *>(controller_))
    {
        return vsc->Eta(p_aero);
    }
    return 0.85;
}
