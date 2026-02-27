#pragma once
/**
 * @file SolverConfig.h
 * @brief Plain data structure carrying all parameters needed by NingSolver.
 *
 * ── Access model ─────────────────────────────────────────────────────────────
 *
 *  OLD (three separate pointers):           NEW (two domain objects):
 *  ─────────────────────────────            ──────────────────────────
 *  InputCase*   input_case                  ISimulationConfig* sim_config
 *  TurbineGeometry* geometry          →     TurbineGeometry*   turbine
 *  vector<Polar>*   polars                  (polars accessed via turbine)
 *
 *  The solver reads blade geometry and polars from turbine and reads
 *  physics constants (ν, a, ε…) from sim_config.  Nothing else changes.
 *
 * Separating configuration from logic lets each class focus on a single job
 * and makes unit-testing trivial (just construct a config struct).
 */
#include <memory>
#include <vector>
#include "ILossModel.h"
#include "IInductionModel.h"
#include "IRootFinder.h"
#include "ISimulationConfig.h"
// #include "Angles.h"

// Forward declarations (the Solver does not need to know their internals).
class TurbineGeometry;
class FlowCalculator;

/**
 * @brief All parameters required to construct and run a NingSolver.
 *
 * All pointers are non-owning (observers).  The objects pointed to must
 * outlive the solver — the same lifetime contract as the original code.
 */
struct SolverConfig
{
    // ── Aerodynamic / physical models (injected via interfaces) ──────────────
    ILossModel const *loss_model{nullptr};           ///< combined tip+hub loss
    IInductionModel const *induction_model{nullptr}; ///< empirical wake model
    IRootFinder const *root_finder{nullptr};         ///< Brent's method etc.

    // ── Domain objects — the two primary access points ────────────────────────
    TurbineGeometry const *turbine{nullptr};      ///< blade geometry + polars
    ISimulationConfig const *sim_config{nullptr}; ///< physics constants + numerics
    FlowCalculator const *flow_calculator{nullptr};

    // ── Scalar operating-point parameters ────────────────────────────────────
    double pitch{0.0}; ///< collective pitch angle [rad]
    double psi{0.0};   ///< azimuth angle [rad]

    // ── Numerical parameters (override sim_config defaults if non-zero) ───────
    double convergence_value{0.0}; ///< 0 → use sim_config->convergence_tolerance()
    unsigned max_iterations{0};    ///< 0 → use default (400)

    // ── Optional features ─────────────────────────────────────────────────────
    bool verbose{false};
};
