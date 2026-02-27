#pragma once
/**
 * @file IBEMOrchestrator.h
 * @brief Abstract interface for classes that orchestrate multiple BEM solves
 *        (one per wind speed / pitch / azimuth combination).
 *
 * Single Responsibility: the orchestrator knows *which* solves to run and in
 * what order. It delegates the actual solving to IBEMSolver instances and
 * post-processing to IPostProcessor.
 *
 * Interface Segregation: opermot only needs RunAll() and results accessors.
 */
#include <cstddef>
#include <vector>

struct SolvePoint
{
    double vinf;
    double lambda;
    double pitch; ///< [deg]
    double psi;   ///< [rad]
};

class IBEMOrchestrator
{
public:
    virtual ~IBEMOrchestrator() = default;

    /**
     * @brief Execute all BEM solves defined by the operating map.
     */
    virtual void RunAll() = 0;

    /**
     * @brief Number of solve points.
     */
    virtual std::size_t Size() const = 0;

    /**
     * @brief Cp at index i (averaged over psi if multi-azimuth).
     */
    virtual double Cp(std::size_t i) const = 0;

    /**
     * @brief Ct at index i.
     */
    virtual double Ct(std::size_t i) const = 0;
};
