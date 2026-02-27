#pragma once
/**
 * @file NingSolverFactory.h
 * @brief Factory that assembles a NingSolver from TurbineGeometry + ISimulationConfig.
 *
 * ── Migration ────────────────────────────────────────────────────────────────
 *
 *  Old signature (three separate pointers):
 *    factory.Build(input_case, geometry, polars, flow_calc, pitch, psi)
 *
 *  New signature (two domain objects):
 *    factory.Build(turbine, sim_config, flow_calc, pitch, psi)
 *
 *  Everything that was split across InputCase / geometry / polars is now
 *  accessed through the two objects you already have in main().
 *
 * Single Responsibility: owns the wiring of concrete physics models into
 * the abstract solver interface.  Call sites never need to know which
 * concrete loss/induction/root-finder model is active.
 *
 * Open/Closed: new solver variants (e.g. Shen tip loss) are added as new
 * Build methods; nothing existing changes.
 */
#include <memory>
#include <vector>
#include "NingSolver.h"
#include "LossModels.h"
#include "EmpiricalWakeInduction.h"
#include "BrentsRootFinder.h"
#include "SolverConfig.h"
#include "ISimulationConfig.h"

class TurbineGeometry;
class FlowCalculator;

class NingSolverFactory
{
public:
    /**
     * @brief Build a standard Ning solver.
     *
     * Physics models used:
     *   - Prandtl combined tip + hub loss
     *   - Ning (2013) empirical wake induction
     *   - Brent's method root finder
     *
     * Numerical constants (wake transition point, convergence tolerance)
     * are read from sim_config so they stay in one place.
     *
     * @param turbine         Blade geometry + polars.  Must outlive the solver.
     * @param sim_config      Physics constants + solver numerics.  Must outlive the solver.
     * @param flow_calculator Pre-built flow field for this (vinf, psi).
     * @param pitch           Collective pitch [rad].
     * @param psi             Rotor azimuth [rad].
     * @param verbose         Log operating point on each Solve() call.
     */
    std::unique_ptr<NingSolver> Build(
        TurbineGeometry const *turbine,
        ISimulationConfig const *sim_config,
        FlowCalculator const *flow_calculator,
        double pitch,
        double psi,
        bool verbose = false)
    {
        auto tip = std::make_unique<PrandtlTipLoss>();
        auto hub = std::make_unique<PrandtlHubLoss>();
        auto combo = std::make_unique<CombinedLoss>(tip.get(), hub.get());
        auto ind = std::make_unique<EmpiricalWakeInduction>(
            sim_config->wake_transition_point());
        auto root = std::make_unique<BrentsRootFinder>(
            sim_config->convergence_tolerance(), 400u);

        owned_tip_.push_back(std::move(tip));
        owned_hub_.push_back(std::move(hub));
        owned_combo_.push_back(std::move(combo));
        owned_ind_.push_back(std::move(ind));
        owned_root_.push_back(std::move(root));

        SolverConfig cfg;
        cfg.loss_model = owned_combo_.back().get();
        cfg.induction_model = owned_ind_.back().get();
        cfg.root_finder = owned_root_.back().get();
        cfg.turbine = turbine;
        cfg.sim_config = sim_config;
        cfg.flow_calculator = flow_calculator;
        cfg.pitch = pitch;
        cfg.psi = psi;
        cfg.verbose = verbose;

        return std::make_unique<NingSolver>(std::move(cfg));
    }

    /**
     * @brief Build a solver with no tip or hub loss (F = 1 everywhere).
     * Useful for clean-sweep validation runs.
     */
    std::unique_ptr<NingSolver> BuildNoLoss(
        TurbineGeometry const *turbine,
        ISimulationConfig const *sim_config,
        FlowCalculator const *flow_calculator,
        double pitch,
        double psi)
    {
        auto no = std::make_unique<NoLoss>();
        auto ind = std::make_unique<EmpiricalWakeInduction>(
            sim_config->wake_transition_point());
        auto root = std::make_unique<BrentsRootFinder>(
            sim_config->convergence_tolerance(), 400u);

        owned_no_loss_.push_back(std::move(no));
        owned_ind_.push_back(std::move(ind));
        owned_root_.push_back(std::move(root));

        SolverConfig cfg;
        cfg.loss_model = owned_no_loss_.back().get();
        cfg.induction_model = owned_ind_.back().get();
        cfg.root_finder = owned_root_.back().get();
        cfg.turbine = turbine;
        cfg.sim_config = sim_config;
        cfg.flow_calculator = flow_calculator;
        cfg.pitch = pitch;
        cfg.psi = psi;

        return std::make_unique<NingSolver>(std::move(cfg));
    }

private:
    std::vector<std::unique_ptr<PrandtlTipLoss>> owned_tip_;
    std::vector<std::unique_ptr<PrandtlHubLoss>> owned_hub_;
    std::vector<std::unique_ptr<CombinedLoss>> owned_combo_;
    std::vector<std::unique_ptr<NoLoss>> owned_no_loss_;
    std::vector<std::unique_ptr<EmpiricalWakeInduction>> owned_ind_;
    std::vector<std::unique_ptr<BrentsRootFinder>> owned_root_;
};
