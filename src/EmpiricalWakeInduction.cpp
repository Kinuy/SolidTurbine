/**
 * @file EmpiricalWakeInduction.cpp
 * @brief Implementation of Ning (2013) empirical wake induction model.
 */
#define _USE_MATH_DEFINES
#include "EmpiricalWakeInduction.h"
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────
EmpiricalWakeInduction::EmpiricalWakeInduction(double wake_transition)
    : x_(wake_transition)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// Compute
// ─────────────────────────────────────────────────────────────────────────────
InductionFactors EmpiricalWakeInduction::Compute(InductionInput const &in) const
{
    InductionFactors out;

    if (in.phi > 0.0)
    {
        out.a_axi = ComputeAxialForwardFlight(in);
    }
    else
    {
        // propeller-brake region
        out.a_axi = (in.k > 1.0) ? in.k / (in.k - 1.0) : 0.0;
    }

    // tangential induction always:
    out.a_rot = in.k_rot / (1.0 - in.k_rot);
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// ComputeAxialForwardFlight
// ─────────────────────────────────────────────────────────────────────────────
double EmpiricalWakeInduction::ComputeAxialForwardFlight(
    InductionInput const &in) const
{
    double k_check = 1.0 / (1.0 / x_ - 1.0);

    if (in.k <= k_check)
    {
        // momentum region:
        return in.k / (1.0 + in.k);
    }

    // empirical region – quadratic formula (Ning 2013 + generalised x):
    double var1 = 2.0 - 4.0 * x_ * in.F * (1.0 - x_);
    double var2 = var1 - (1.0 - x_) * 4.0 * in.F * (1.0 - 2.0 * x_);
    double var3 = (1.0 - x_ * x_) - (1.0 - x_) * 2.0 * x_;

    double b2 = var2 / var3;
    double b1 = 4.0 * in.F * (1.0 - 2.0 * x_) - 2.0 * x_ * b2;
    double b0 = 2.0 - b1 - b2;

    double qa = b2 - 4.0 * in.F * in.k;
    double qb = b1 + 8.0 * in.F * in.k;
    double qc = b0 - 4.0 * in.F * in.k;

    constexpr double eps = 1e-8;
    if (std::abs(qa) < eps)
        qa = eps;

    return (-qb + std::sqrt(qb * qb - 4.0 * qa * qc)) / (2.0 * qa);
}
