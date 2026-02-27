#pragma once
/**
 * @file VariableSpeedController.h
 * @brief Variable-speed, variable-pitch turbine controller.
 */
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <string>
#include <map>
#include <stdexcept>
#include "ITurbineController.h"
#include "IPitchSchedule.h"
#include "IEfficiencyModel.h"
#include "ISimulationConfig.h"
#include "MathUtilities.h"

class TurbineGeometry;

struct TurbineControllerParams
{
    double p_max;
    double n_soll;
    double n_nenn;
    double n_min;
    double lambda_opt;
    double max_dp_dn;
    double rotor_radius;
    double v_min;
    double v_max;
    std::string power_mode;

    std::map<double, double> bkl_n_to_pel;
    std::map<double, double> bkl_pel_to_n;

    static TurbineControllerParams FromConfig(
        TurbineGeometry const *turbine,
        ISimulationConfig const *sim_config);
};

// ─────────────────────────────────────────────────────────────────────────────

class VariableSpeedController final : public ITurbineController
{
public:
    VariableSpeedController(TurbineControllerParams params,
                            IPitchSchedule const *pitch_schedule,
                            IEfficiencyModel const *efficiency_model);

    std::string Name() const override;
    ControllerOutput ComputeOperatingPoint(ControllerInput const &in) const override;

    double Eta(double p_mech) const;
    double GetNOfBKL(double p_el) const;
    double GetPelOfBKL(double n) const;

private:
    TurbineControllerParams p_;
    IPitchSchedule const *pitch_;
    IEfficiencyModel const *eta_model_;

    double ComputeSpeedL0Mode(ControllerInput const &in) const;
};