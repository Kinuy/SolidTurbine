#pragma once
/**
 * @file EfficiencyModels.h
 * @brief Concrete drivetrain efficiency implementations.
 *
 * Open/Closed: new models added here; existing code unchanged.
 */
#include <map>
#include <stdexcept>
#include "IEfficiencyModel.h"
#include "MathUtilities.h" // WVPMUtilities::linear_interpolation

// ─────────────────────────────────────────────────────────────────────────────
// Constant efficiency (useful for unit tests and simple cases).
// ─────────────────────────────────────────────────────────────────────────────
class ConstantEfficiency final : public IEfficiencyModel
{
public:
    explicit ConstantEfficiency(double eta = 0.85) : eta_(eta) {}
    double Efficiency(double /*p_mech*/) const override { return eta_; }

private:
    double eta_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Table-lookup efficiency (piecewise-linear interpolation on P_mech vs η).
// ─────────────────────────────────────────────────────────────────────────────
class TableEfficiency final : public IEfficiencyModel
{
public:
    /**
     * @param pmech_vec  Mechanical power breakpoints [W].
     * @param eta_vec    Efficiency values at each breakpoint.
     */
    TableEfficiency(std::vector<double> const &pmech_vec,
                    std::vector<double> const &eta_vec)
        : table_(WVPMUtilities::fill_for_linear_interpolation(pmech_vec, eta_vec)) {}

    double Efficiency(double p_mech) const override
    {
        return WVPMUtilities::linear_interpolation(p_mech, table_);
    }

private:
    std::map<double, double> table_;
};
