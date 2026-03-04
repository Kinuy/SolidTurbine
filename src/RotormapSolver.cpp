/**
 * @file RotormapSolver.cpp
 * @brief Implementation of RotormapSolver.
 */
#define _USE_MATH_DEFINES
#include "RotormapSolver.h"
#include "FlowCalculator.h"

#include <cmath>
#include <iostream>
#include <numbers>
#include <stdexcept>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ─────────────────────────────────────────────────────────────────────────────
RotormapSolver::RotormapSolver(TurbineGeometry const   *turbine,
                               ISimulationConfig const *sim_config)
    : turbine_(turbine), sim_config_(sim_config)
{
    if (!turbine_)
        throw std::invalid_argument("RotormapSolver: turbine must be non-null");
    if (!sim_config_)
        throw std::invalid_argument("RotormapSolver: sim_config must be non-null");
}

// ─────────────────────────────────────────────────────────────────────────────
RotormapResult RotormapSolver::Solve(RotormapParams const &params) const
{
    RotormapResult result;
    result.v_tip      = params.v_tip;
    result.lambda_vec = BuildRange(params.lambda_start,
                                   params.lambda_end,
                                   params.lambda_step);
    result.pitch_vec  = BuildRange(params.pitch_start,
                                   params.pitch_end,
                                   params.pitch_step);

    const int I = result.count_I();
    const int J = result.count_J();
    result.points.reserve(static_cast<std::size_t>(I * J));

    int converged = 0;
    const int total = I * J;

    std::cout << "  Rotormap: " << J << " pitch x " << I
              << " lambda = " << total << " points  (v_tip="
              << params.v_tip << " m/s)\n";

    for (int j = 0; j < J; ++j)
        for (int i = 0; i < I; ++i)
        {
            RotormapPoint pt = SolvePoint(result.pitch_vec[j],
                                          result.lambda_vec[i],
                                          params.v_tip);
            if (pt.converged) ++converged;
            result.points.push_back(std::move(pt));
        }

    std::cout << "  Rotormap: " << converged << "/" << total
              << " points converged\n";
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// SolvePoint — mirrors the BEM loop in main.cpp exactly:
//   fc_factory_.Build  →  solver_factory_.Build  →  Solve
//   →  BEMPostprocessor constructed with same fc  →  Process
//
// fc must outlive both solver and postproc, so all three live on this frame.
// ─────────────────────────────────────────────────────────────────────────────
RotormapPoint RotormapSolver::SolvePoint(double pitch_rad,
                                          double lambda,
                                          double v_tip) const
{
    RotormapPoint pt;
    pt.pitch_rad = pitch_rad;
    pt.lambda    = lambda;
    pt.v_tip     = v_tip;

    if (lambda <= 0.0)
    {
        std::cerr << "  RotormapSolver: skipping lambda=" << lambda
                  << " (must be > 0)\n";
        return pt;
    }

    pt.v_inf = v_tip / lambda;

    const double R        = turbine_->RotorRadius();
    const double rot_rate = (R > 0.0) ? v_tip / R : 0.0;
    try
    {
        // ── Flow field (must outlive solver and postproc) ─────────────────────
        auto fc = fc_factory_.Build(turbine_, rot_rate, pt.v_inf,
                                    /*psi=*/0.0, FlowModifiers{});

        // ── BEM solve ─────────────────────────────────────────────────────────
        auto solver = solver_factory_.Build(turbine_, sim_config_,
                                            fc.get(), pitch_rad, /*psi=*/0.0);

        if (!solver->Solve())
            return pt; // not converged — zero-filled result

        // ── Postprocessing ────────────────────────────────────────────────────
        BEMPostprocessor postproc(turbine_,
                                  sim_config_,
                                  fc.get(),
                                  static_cast<double>(turbine_->num_blades()));
        postproc.Process(*solver);

        if (!postproc.Success())
            return pt;

        pt.pp        = postproc.Result();
        pt.converged = true;
    }
    catch (std::exception const &e)
    {
        std::cerr << "  RotormapSolver: exception at pitch_deg=" << pitch_rad * 180.0 / M_PI
                  << " lambda=" << lambda << ": " << e.what() << '\n';
    }

    return pt;
}

// ─────────────────────────────────────────────────────────────────────────────
std::vector<double> RotormapSolver::BuildRange(double start,
                                                double end,
                                                double step)
{
    if (step <= 0.0)
        throw std::invalid_argument(
            "RotormapSolver::BuildRange: step must be > 0");

    std::vector<double> vec;
    for (double v = start; v <= end + step * 1e-9; v += step)
        vec.push_back(v);

    return vec;
}
