#pragma once
/**
 * @file IBEMSolver.h
 * @brief Abstract interface for BEM (Blade Element Momentum) solvers.
 *
 * Dependency Inversion Principle: High-level modules (opermot, time-based solve)
 * depend on this abstraction, not on the concrete Solver implementation.
 */
#include <cstddef>
#include <vector>
// #include "Angles.h"

/**
 * @brief Result of a completed BEM solve for one operating point.
 */
struct SolverResult
{
    bool success{false};
    std::vector<double> phi;       ///< flow angle per section [rad]
    std::vector<double> a_ind_axi; ///< axial induction factor per section
    std::vector<double> a_ind_rot; ///< tangential induction factor per section
    double v_inf{0.0};
    double lambda{0.0};
    double pitch{0.0}; ///< [rad]
    double psi{0.0};   ///< azimuth [rad]
    double cp{0.0};    ///< power coefficient [-]
    double ct{0.0};    ///< thrust coefficient [-]
};

/**
 * @brief Pure interface for all BEM solvers.
 *
 * Interface Segregation: only the minimal contract needed by consumers.
 */
class IBEMSolver
{
public:
    virtual ~IBEMSolver() = default;

    /// Execute the solve. Returns true on success.
    virtual bool Solve() = 0;

    /// Query the result (valid only after a successful Solve()).
    virtual SolverResult const &Result() const = 0;

    /// Convenience helpers — implemented in terms of Result().
    bool SolutionWasSuccessful() const { return Result().success; }
    virtual double LocalFlowVel(std::size_t section) const = 0;
    virtual double LocalReynoldsNumber(std::size_t section) const = 0;
    virtual double LocalMachNumber(std::size_t section) const = 0;
};