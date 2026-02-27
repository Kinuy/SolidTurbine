#pragma once
/**
 * @file IBEMPostprocessor.h
 * @brief Abstract interface for BEM postprocessing.
 *
 * SRP: postprocessing is separated from the BEM solve loop.
 * DIP: consumers depend on this abstraction, not the concrete implementation.
 * OCP: new postprocessing strategies can be added by subclassing.
 */
#include "IBEMSolver.h"

class IBEMPostprocessor
{
public:
    virtual ~IBEMPostprocessor() = default;

    /// Run all postprocessing steps. Valid only after a successful IBEMSolver::Solve().
    virtual void Process(IBEMSolver const &solver) = 0;

    /// True after a successful Process() call.
    virtual bool Success() const = 0;
};
