#pragma once
/**
 * @file EmpiricalWakeInduction.h
 * @brief Induction model implementing Ning (2013) with configurable
 *        empirical wake transition point.
 *
 * Single Responsibility: owns only the induction-factor computation.
 * This is the same math as the original NingComputeInductionFactors()
 * but extracted into its own class.
 */
#include "IInductionModel.h"

class EmpiricalWakeInduction final : public IInductionModel
{
public:
    /**
     * @param wake_transition  Crossover axial induction at which the empirical
     *                         wake correction activates.
     *                         0.4     → same as Ning (2013)
     *                         0.3539  → same as Bladed
     */
    explicit EmpiricalWakeInduction(double wake_transition = 0.4);

    InductionFactors Compute(InductionInput const &in) const override;

private:
    double x_; ///< empirical wake transition point

    double ComputeAxialForwardFlight(InductionInput const &in) const;
};
