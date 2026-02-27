#pragma once
/**
 * @file OperationSolver.h
 * @brief Outer control loop that iterates over (vinf, pitch) operating points
 *        and builds the power curve.
 */
#define _USE_MATH_DEFINES
#include <cmath>
#include <functional>
#include <vector>
#include <iostream>
#include "ITurbineController.h"
#include "ISimulationConfig.h"

class TurbineGeometry;
class VariableSpeedController;

// ─────────────────────────────────────────────────────────────────────────────

struct PowerCurvePoint
{
    double vinf;
    double vtip;
    double pitch; ///< [deg]
    double lambda;
    double p_wind; ///< [W]
    double cp_aero;
    double p_aero; ///< [W]
    double n;      ///< [rpm]
    double torque; ///< [Nm]
    double eta;
    double p_el; ///< [W]
    double ct;
};

/// Callback invoked for each (vinf, lambda, pitch) triple. Returns {cp, ct}.
using BEMCallback = std::function<std::pair<double, double>(double vinf,
                                                            double lambda,
                                                            double pitch_deg)>;

// ─────────────────────────────────────────────────────────────────────────────

struct OperationSolverParams
{
    double air_density;
    double rotor_radius;
    double p_max;
    int min_iter{3};  ///< minimum iterations before convergence check
    int max_iter{50}; ///< maximum iterations before forced exit

    static OperationSolverParams FromConfig(
        TurbineGeometry const *turbine,
        ISimulationConfig const *sim_config);
};

// ─────────────────────────────────────────────────────────────────────────────

class OperationSolver
{
public:
    OperationSolver(OperationSolverParams params,
                    ITurbineController const *controller,
                    BEMCallback bem_callback);

    /// Run the power-curve loop for one pitch setting and all wind speeds.
    std::vector<PowerCurvePoint> Run(double pitch_deg,
                                     std::vector<double> const &vinf_vec);

private:
    OperationSolverParams p_;
    ITurbineController const *controller_;
    BEMCallback bem_;

    /// Inner convergence loop for a single wind speed.
    /// Updates vtip_inout as a warm-start for the next wind speed.
    PowerCurvePoint ConvergeOnePoint(double vinf,
                                     double &vtip_inout,
                                     double pitch_deg);

    /// Populate all fields of a PowerCurvePoint in one call.
    void FillResult(PowerCurvePoint &pt,
                    double vtip, double lambda, double pitch,
                    double cp, double p_aero,
                    double n_rpm, double torque,
                    double eta, double p_el, double ct) const;

    /// Query drivetrain efficiency from the controller if it exposes Eta().
    /// Falls back to 0.85 for controllers that do not implement it.
    double EtaFromController(double p_aero) const;
};
