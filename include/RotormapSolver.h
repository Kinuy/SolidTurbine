#pragma once
/**
 * @file RotormapSolver.h
 * @brief Computes a pitch x lambda performance map (Rotormap).
 *
 * SOLID compliance:
 *  S - responsible only for the pitch x lambda double loop and collecting
 *      BEMPostprocessResult per point; export is fully delegated.
 *  O - extended via IRotormapExporter without modifying this class.
 *  I - callers receive a RotormapResult; solver internals are hidden.
 *  D - depends on TurbineGeometry, ISimulationConfig, and factory
 *      abstractions, not on NingSolver or FlowCalculator directly.
 *
 * Operating mode: constant v_tip sweep
 *
 *   v_inf    = v_tip / lambda
 *   rot_rate = v_tip / R   (constant for the entire map)
 *
 * Outer loop: pitch (J-direction), inner loop: lambda (I-direction).
 * Points stored row-major: points[j * I + i].
 */
#include "BEMPostprocessor.h"
#include "FlowCalculatorFactory.h"
#include "NingSolverFactory.h"
#include "ISimulationConfig.h"
#include "TurbineGeometry.h"

#include <vector>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
/// Parameters driving the Rotormap sweep.
struct RotormapParams
{
    double v_tip{0.0};        ///< Constant tip speed [m/s]
    double lambda_start{0.0}; ///< TSR sweep start [-]
    double lambda_end{0.0};   ///< TSR sweep end   [-]
    double lambda_step{1.0};  ///< TSR sweep step  [-]
    double pitch_start{0.0};  ///< Pitch sweep start [rad]
    double pitch_end{0.0};    ///< Pitch sweep end   [rad]
    double pitch_step{1.0};   ///< Pitch sweep step  [rad]
};

// ─────────────────────────────────────────────────────────────────────────────
/// One solved point in the Rotormap.
struct RotormapPoint
{
    double pitch_rad{0.0}; ///< Pitch angle [rad]
    double lambda{0.0};    ///< Tip-speed ratio [-]
    double v_tip{0.0};     ///< Tip speed [m/s]
    double v_inf{0.0};     ///< Wind speed [m/s]
    bool   converged{false};
    BEMPostprocessResult pp; ///< Full postprocessor result (zeroed if not converged)
};

// ─────────────────────────────────────────────────────────────────────────────
/**
 * @brief Complete Rotormap result - the 2-D grid of solved points.
 *
 * Stored row-major (Tecplot DATAPACKING=POINT convention):
 *   outer = pitch (J),  inner = lambda (I)
 *   Access: points[j * lambda_vec.size() + i]
 */
struct RotormapResult
{
    std::vector<double>        lambda_vec; ///< All lambda values (I-direction)
    std::vector<double>        pitch_vec;  ///< All pitch values  (J-direction)
    double                     v_tip{0.0};
    std::vector<RotormapPoint> points;

    int count_I() const { return static_cast<int>(lambda_vec.size()); }
    int count_J() const { return static_cast<int>(pitch_vec.size()); }
};

// ─────────────────────────────────────────────────────────────────────────────
/// Drives the pitch x lambda sweep and collects BEM results.
class RotormapSolver
{
public:
    RotormapSolver(TurbineGeometry const   *turbine,
                   ISimulationConfig const *sim_config);

    RotormapResult Solve(RotormapParams const &params) const;

private:
    TurbineGeometry const   *turbine_;
    ISimulationConfig const *sim_config_;

    mutable FlowCalculatorFactory fc_factory_;
    mutable NingSolverFactory     solver_factory_;

    RotormapPoint SolvePoint(double pitch_rad,
                             double lambda,
                             double v_tip) const;

    static std::vector<double> BuildRange(double start,
                                          double end,
                                          double step);
};
