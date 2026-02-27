/**
 * @file BEMPostprocessor.cpp
 * @brief Implementation of BEMPostprocessor.
 *
 * Physics adapted directly from the legacy wvp::PostProcess class,
 * refactored to SOLID principles:
 *  - No dependency on legacy InputCase, Solver, or Polar types.
 *  - All polar lookups delegated to TurbineGeometry::InterpForCoeff().
 *  - All velocity data fetched from FlowCalculator and IBEMSolver helpers.
 */
#define _USE_MATH_DEFINES
#include "BEMPostprocessor.h"
#include "TurbineGeometry.h"
#include "FlowCalculator.h"
#include "ISimulationConfig.h"

#include <cmath>
#include <stdexcept>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────
BEMPostprocessor::BEMPostprocessor(TurbineGeometry const *turbine,
                                   ISimulationConfig const *sim_config,
                                   FlowCalculator const *flow_calc,
                                   double num_blades)
    : turbine_(turbine), sim_config_(sim_config), flow_calc_(flow_calc), num_blades_(num_blades)
{
    if (!turbine_)
        throw std::invalid_argument("BEMPostprocessor: turbine must be non-null");
    if (!sim_config_)
        throw std::invalid_argument("BEMPostprocessor: sim_config must be non-null");
    if (!flow_calc_)
        throw std::invalid_argument("BEMPostprocessor: flow_calc must be non-null");
}

// ─────────────────────────────────────────────────────────────────────────────
// Public entry point
// ─────────────────────────────────────────────────────────────────────────────
void BEMPostprocessor::Process(IBEMSolver const &solver)
{
    success_ = false;
    result_ = {};

    if (!solver.SolutionWasSuccessful())
        return;

    n_sec_ = turbine_->num_sections();
    AllocateArrays();

    ComputeElementLengths(*turbine_);
    ComputeLocalFlowAngles(solver);
    ComputeLocalElementLoads(solver);
    ComputePowerAndThrust(solver);
    ComputeFullBladeMoments();
    ComputeIntegratedLoads();

    success_ = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// AllocateArrays
// ─────────────────────────────────────────────────────────────────────────────
void BEMPostprocessor::AllocateArrays()
{
    auto &r = result_;
    r.alpha_eff.assign(n_sec_, 0.0);
    r.cl.assign(n_sec_, 0.0);
    r.cd.assign(n_sec_, 0.0);
    r.cm.assign(n_sec_, 0.0);
    r.cp_loc.assign(n_sec_, 0.0);
    r.ct_loc.assign(n_sec_, 0.0);
    r.element_length.assign(n_sec_, 0.0);
    r.element_thrust.assign(n_sec_, 0.0);
    r.element_torque.assign(n_sec_, 0.0);
    r.element_fy.assign(n_sec_, 0.0);
    r.element_mz.assign(n_sec_, 0.0);
    r.element_airfoil_moment.assign(n_sec_, 0.0);
    r.integral_fx.assign(n_sec_, 0.0);
    r.integral_fy.assign(n_sec_, 0.0);
    r.integral_mx.assign(n_sec_, 0.0);
    r.integral_my.assign(n_sec_, 0.0);
    r.integral_mz.assign(n_sec_, 0.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// ComputeElementLengths  (trapezoidal dr)
// ─────────────────────────────────────────────────────────────────────────────
void BEMPostprocessor::ComputeElementLengths(TurbineGeometry const &tg)
{
    auto &dr = result_.element_length;
    for (std::size_t i = 0; i < n_sec_; ++i)
    {
        if (i == 0)
            dr[i] = (tg.radius(1) - tg.radius(0)) / 2.0;
        else if (i == n_sec_ - 1)
            dr[i] = (tg.radius(i) - tg.radius(i - 1)) / 2.0;
        else
            dr[i] = (tg.radius(i + 1) - tg.radius(i - 1)) / 2.0;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ComputeLocalFlowAngles
// alpha_eff = phi - (twist + pitch + torsion)   [all in rad]
// ─────────────────────────────────────────────────────────────────────────────
void BEMPostprocessor::ComputeLocalFlowAngles(IBEMSolver const &solver)
{
    SolverResult const &res = solver.Result();
    // double v_tip = res.lambda * res.v_inf; // [m/s]

    for (std::size_t i = 0; i < n_sec_; ++i)
    {
        double twist = turbine_->twist(i);             // [deg]
        double pitch = static_cast<double>(res.pitch); // [deg]
        // double torsion  = turbine_->TorsionAtR(i, v_tip, res.v_inf);       // [rad]
        double beta = twist + pitch; //+ torsion;

        result_.alpha_eff[i] = res.phi[i] - beta;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ComputeLocalElementLoads
// dT, dQ, dFy, dMz per section + local Cl, Cd, Cm, cp_loc, ct_loc
// ─────────────────────────────────────────────────────────────────────────────
void BEMPostprocessor::ComputeLocalElementLoads(IBEMSolver const &solver)
{
    SolverResult const &res = solver.Result();
    double v_inf = res.v_inf;
    double rho = sim_config_->air_density();
    double rotation_rate = (turbine_->RotorRadius() > 0.0)
                               ? res.lambda * v_inf / turbine_->RotorRadius()
                               : 0.0;

    for (std::size_t i = 0; i < n_sec_; ++i)
    {
        // ── Polar lookup ──────────────────────────────────────────────────────
        double Cl{0.0}, Cd{0.0}, Cm{0.0};
        turbine_->InterpForCoeff(i,
                                 solver.LocalReynoldsNumber(i),
                                 solver.LocalMachNumber(i),
                                 result_.alpha_eff[i],
                                 &Cl, &Cd, &Cm);

        result_.cl[i] = Cl;
        result_.cd[i] = Cd;
        result_.cm[i] = Cm;

        // ── Aerodynamic loading ───────────────────────────────────────────────
        double loc_vel = solver.LocalFlowVel(i);
        double chord = turbine_->chord(i);
        double dr = result_.element_length[i];
        double phi = res.phi[i];

        double denom = 0.5 * rho * loc_vel * loc_vel * chord * dr;
        double lift = Cl * denom;
        double drag = Cd * denom;
        double moment = Cm * denom * chord; // [Nm]

        double dT = lift * std::cos(phi) + drag * std::sin(phi);                         // thrust
        double dFy = -(lift * std::sin(phi) - drag * std::cos(phi));                     // in-plane
        double dQ = (lift * std::sin(phi) - drag * std::cos(phi)) * turbine_->radius(i); // torque

        result_.element_thrust[i] = dT;
        result_.element_fy[i] = dFy;
        result_.element_torque[i] = dQ;
        result_.element_airfoil_moment[i] = moment;

        // ── Section torsion moment ────────────────────────────────────────────
        result_.element_mz[i] = dFy * turbine_->aeroCentreX(i) - dT * turbine_->aeroCentreY(i) + moment;

        // ── Local Cp and Ct coefficients ──────────────────────────────────────
        double r = turbine_->radius(i);
        double v3 = v_inf * v_inf * v_inf;
        double v2 = v_inf * v_inf;
        double pi_r = M_PI * r;

        // cp_loc: per IEC annular element definition
        result_.cp_loc[i] = (dr > 0.0 && v3 > 0.0)
                                ? num_blades_ * dQ * rotation_rate / (dr * rho * v3 * pi_r)
                                : 0.0;

        // ct_loc: per IEC annular element definition
        result_.ct_loc[i] = (dr > 0.0 && v2 > 0.0)
                                ? num_blades_ * dT / (dr * rho * v2 * pi_r)
                                : 0.0;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ComputePowerAndThrust  (rotor-integrated)
// ─────────────────────────────────────────────────────────────────────────────
void BEMPostprocessor::ComputePowerAndThrust(IBEMSolver const &solver)
{
    SolverResult const &res = solver.Result();
    double v_inf = res.v_inf;
    double rho = sim_config_->air_density();
    double R = turbine_->RotorRadius();
    double rotation_rate = (R > 0.0) ? res.lambda * v_inf / R : 0.0;

    double turbine_area = M_PI * R * R;
    double pow_denom = 0.5 * rho * v_inf * v_inf * v_inf * turbine_area;
    double thr_denom = 0.5 * rho * v_inf * v_inf * turbine_area;
    double torque_denom = thr_denom * R;

    for (std::size_t i = 0; i < n_sec_; ++i)
    {
        double dT = result_.element_thrust[i];
        double dQ = result_.element_torque[i];
        double dFy = result_.element_fy[i];

        result_.p += num_blades_ * dQ * rotation_rate;
        result_.cp += (pow_denom > 0.0) ? num_blades_ * dQ * rotation_rate / pow_denom : 0.0;
        result_.thrust += num_blades_ * dT;
        result_.ct += (thr_denom > 0.0) ? num_blades_ * dT / thr_denom : 0.0;
        result_.torque += num_blades_ * dQ;
        result_.ctorque += (torque_denom > 0.0) ? num_blades_ * dQ / torque_denom : 0.0;
        result_.sum_fy += dFy;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ComputeFullBladeMoments  (single blade root moments)
// ─────────────────────────────────────────────────────────────────────────────
void BEMPostprocessor::ComputeFullBladeMoments()
{
    for (std::size_t i = 0; i < n_sec_; ++i)
    {
        result_.mx += result_.element_torque[i];
        result_.my += result_.element_thrust[i] * turbine_->radius(i);
        result_.mz += result_.element_mz[i];
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ComputeIntegratedLoads  (cumulative from tip inward to each section)
// ─────────────────────────────────────────────────────────────────────────────
void BEMPostprocessor::ComputeIntegratedLoads()
{
    for (std::size_t i = 0; i < n_sec_; ++i)
    {
        double it_fx = 0.0, it_fy = 0.0;
        double it_mx = 0.0, it_my = 0.0, it_mz = 0.0;

        for (std::size_t j = i; j < n_sec_; ++j)
        {
            double dist = turbine_->radius(j) - turbine_->radius(i);

            it_fx += result_.element_thrust[j];
            it_fy += result_.element_fy[j];
            it_mx += result_.element_fy[j] * (-dist);
            it_my += result_.element_thrust[j] * dist;
            it_mz += result_.element_mz[j];
        }

        result_.integral_fx[i] = it_fx;
        result_.integral_fy[i] = it_fy;
        result_.integral_mx[i] = it_mx;
        result_.integral_my[i] = it_my;
        result_.integral_mz[i] = it_mz;
    }
}
