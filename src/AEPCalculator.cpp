/**
 * @file AEPCalculator.cpp
 */
#define _USE_MATH_DEFINES
#include "AEPCalculator.h"

#include <cmath>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
AEPCalculator::AEPCalculator(std::vector<double> vinf_vec,
                             std::vector<double> pel_vec,
                             double dv,
                             double k,
                             double money_per_kwh)
    : vinf_(std::move(vinf_vec)), pel_(std::move(pel_vec)), dv_(dv), k_(k), money_(money_per_kwh)
{
    if (vinf_.size() != pel_.size())
        throw std::invalid_argument("AEPCalculator: vinf and pel must be same length");
}

// ─────────────────────────────────────────────────────────────────────────────
AEPCalculator::AEPResult AEPCalculator::Compute(double v_mean) const
{
    double A = v_mean / std::tgamma(1.0 + 1.0 / k_);
    double W = 0.0;

    for (std::size_t i = 0; i < vinf_.size(); ++i)
    {
        double D = WeibullBin(vinf_[i], dv_, A, k_);
        W += D * pel_[i] / 1000.0; // W → kW → kWh/a
    }

    return {W, W * money_};
}

std::vector<AEPCalculator::AEPResult> AEPCalculator::ComputeRange(
    std::vector<double> const &vmean_vec) const
{
    std::vector<AEPResult> results;
    results.reserve(vmean_vec.size());
    for (double v_mean : vmean_vec)
        results.push_back(Compute(v_mean));
    return results;
}

// ─────────────────────────────────────────────────────────────────────────────
double AEPCalculator::WeibullBin(double v, double dv, double A, double k)
{
    return 24.0 * 365.0 *
           (std::exp(-std::pow((v - dv) / A, k)) -
            std::exp(-std::pow((v + dv) / A, k)));
}