/**
 * @file VariableSpeedController.cpp
 */
#define _USE_MATH_DEFINES
#include "VariableSpeedController.h"
#include "TurbineGeometry.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// TurbineControllerParams::FromConfig
// ─────────────────────────────────────────────────────────────────────────────
TurbineControllerParams TurbineControllerParams::FromConfig(
    TurbineGeometry const *turbine,
    ISimulationConfig const *sim_config)
{
    if (!turbine)
        throw std::invalid_argument("TurbineControllerParams: turbine is null");
    if (!sim_config)
        throw std::invalid_argument("TurbineControllerParams: sim_config is null");

    TurbineControllerParams p;
    p.p_max = sim_config->rated_power();
    p.n_soll = sim_config->speed_setpoint_rpm();
    p.n_nenn = sim_config->max_speed_rpm();
    p.n_min = sim_config->min_speed_rpm();
    p.lambda_opt = sim_config->optimal_tsr();
    p.max_dp_dn = sim_config->max_power_speed_gradient();
    p.rotor_radius = turbine->RotorRadius();
    p.v_min = sim_config->wind_speed_start();
    p.v_max = sim_config->wind_speed_end();
    p.power_mode = sim_config->power_mode();
    // BKL tables are not in ISimulationConfig — must be set separately
    // p.bkl_n_to_pel and p.bkl_pel_to_n remain empty by default
    return p;
}

// ─────────────────────────────────────────────────────────────────────────────
// VariableSpeedController
// ─────────────────────────────────────────────────────────────────────────────
VariableSpeedController::VariableSpeedController(
    TurbineControllerParams params,
    IPitchSchedule const *pitch_schedule,
    IEfficiencyModel const *efficiency_model)
    : p_(std::move(params)), pitch_(pitch_schedule), eta_model_(efficiency_model)
{
    if (!pitch_ || !eta_model_)
        throw std::invalid_argument(
            "VariableSpeedController: pitch and efficiency dependencies must be non-null");
}

// ─────────────────────────────────────────────────────────────────────────────
std::string VariableSpeedController::Name() const
{
    return "VariableSpeedController";
}

// ─────────────────────────────────────────────────────────────────────────────
ControllerOutput VariableSpeedController::ComputeOperatingPoint(
    ControllerInput const &in) const
{
    ControllerOutput out;

    if (in.Pel >= p_.p_max)
    {
        out.n = p_.n_soll;
    }
    else if (p_.power_mode == "L0")
    {
        out.n = ComputeSpeedL0Mode(in);
    }
    else
    {
        out.n = WVPMUtilities::linear_interpolation(in.Pel, p_.bkl_pel_to_n);
        out.n = std::max(out.n, p_.n_min);
    }

    out.n = std::clamp(out.n, p_.n_min, p_.n_nenn);
    out.vtip = out.n / 60.0 * 2.0 * M_PI * p_.rotor_radius;
    out.lambda = (in.vinf > 0.0) ? out.vtip / in.vinf : 0.0;
    out.pitch = pitch_->BasePitch() + pitch_->PitchOffset(in.Pel);
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
double VariableSpeedController::Eta(double p_mech) const
{
    return eta_model_->Efficiency(p_mech);
}

double VariableSpeedController::GetNOfBKL(double p_el) const
{
    return WVPMUtilities::linear_interpolation(p_el, p_.bkl_pel_to_n);
}

double VariableSpeedController::GetPelOfBKL(double n) const
{
    return WVPMUtilities::linear_interpolation(n, p_.bkl_n_to_pel);
}

// ─────────────────────────────────────────────────────────────────────────────
double VariableSpeedController::ComputeSpeedL0Mode(ControllerInput const &in) const
{
    double v_tip = p_.lambda_opt * in.vinf;
    double n = 30.0 * v_tip / (p_.rotor_radius * M_PI);

    if (n >= p_.n_nenn)
        n = p_.n_nenn - 0.001;

    double grad = (p_.p_max - in.Pel) / (p_.n_nenn - n);
    if (grad > p_.max_dp_dn)
        n = p_.n_nenn - (p_.p_max - in.Pel) / p_.max_dp_dn;

    return n;
}