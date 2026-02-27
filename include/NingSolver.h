#pragma once
/**
 * @file NingSolver.h
 * @brief Clean, SOLID-compliant BEM solver using the Ning (2013) algorithm.
 *
 * SOLID compliance:
 *  S – Responsible only for solving BEM equations per section.
 *  O – Extensible via injected ILossModel / IInductionModel / IRootFinder.
 *  L – Satisfies IBEMSolver contract completely.
 *  I – Exposes only what consumers need through IBEMSolver.
 *  D – Depends on abstractions (interfaces), not concrete classes.
 *
 * Reference: Ning (2013) "A simple solution for the blade element momentum
 * equations with guaranteed convergence." Wind Energy. DOI:10.1002/we.1636
 */
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include "IBEMSolver.h"
#include "SolverConfig.h"
// #include "Angles.h"

// Forward declarations – full headers included only in the .cpp
class TurbineGeometry;
class InputCase;
class Polar;
class FlowCalculator;

class NingSolver final : public IBEMSolver
{
public:
    // ── Construction ────────────────────────────────────────────────────────
    explicit NingSolver(SolverConfig cfg);
    ~NingSolver() override = default;

    // Non-copyable (large internal state, use unique_ptr at call sites).
    NingSolver(NingSolver const &) = delete;
    NingSolver &operator=(NingSolver const &) = delete;
    NingSolver(NingSolver &&) = default;
    NingSolver &operator=(NingSolver &&) = default;

    // ── IBEMSolver interface ─────────────────────────────────────────────────
    bool Solve() override;
    SolverResult const &Result() const override { return result_; }

    // ── Read-only accessors (used by PostProcess / output) ───────────────────
    double v_inf() const;
    double lambda() const;
    double pitch() const { return cfg_.pitch; }
    double psi() const { return cfg_.psi; }

    double LocalFlowVel(std::size_t section) const;
    double LocalReynoldsNumber(std::size_t section) const;
    double LocalMachNumber(std::size_t section) const;

    std::vector<double> const &phi() const { return result_.phi; }
    std::vector<double> const &a_ind_axi() const { return result_.a_ind_axi; }
    std::vector<double> const &a_ind_rot() const { return result_.a_ind_rot; }

private:
    // ── Configuration (injected dependencies) ────────────────────────────────
    SolverConfig cfg_;

    // ── Mutable solve state ──────────────────────────────────────────────────
    SolverResult result_;
    std::vector<double> sec_solidity_; ///< local solidity per section
    std::vector<double> beta_;         ///< effective twist+pitch [rad]
    std::vector<double> k_cache_;      ///< latest k value per section
    std::vector<int> converged_;       ///< 1 = converged, 0 = not

    std::size_t num_sections_{0};

    // ── Private algorithm steps ──────────────────────────────────────────────
    void Initialise();
    void SolveAllSections();
    void FinaliseResult();

    // Per-section search
    bool FindSolutionPositiveRegion(std::size_t sec);
    bool FindSolutionNegativeRegion(std::size_t sec);

    // Root-bracketing (zbrak equivalent)
    std::vector<std::pair<double, double>> FindBrackets(double x1,
                                                        double x2,
                                                        std::size_t sec) const;

    // Residual function f(phi) for section `sec`.
    double Residual(double phi, std::size_t sec);

    // Solve polar + induction for one residual evaluation.
    void EvaluatePolarAndInduction(double phi,
                                   std::size_t sec,
                                   double &k_out,
                                   double &k_rot_out);

    // Logging
    void LogOperatingPoint() const;
    void LogFailedSections() const;
};
