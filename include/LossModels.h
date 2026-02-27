#pragma once
/**
 * @file LossModels.h
 * @brief Concrete implementations of ILossModel.
 *
 * Open/Closed: new models are added here without changing the Solver.
 * Single Responsibility: each class is responsible for exactly one loss formula.
 */
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <memory>
#include "ILossModel.h"

// ─────────────────────────────────────────────────────────────────────────────
// No loss (F = 1 always). Used when tip/hub loss is disabled.
// ─────────────────────────────────────────────────────────────────────────────
class NoLoss final : public ILossModel
{
public:
    double Evaluate(LossModelInput const & /*input*/) const override
    {
        return 1.0;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Classic Prandtl tip-loss factor (F_T).
// ─────────────────────────────────────────────────────────────────────────────
class PrandtlTipLoss final : public ILossModel
{
public:
    double Evaluate(LossModelInput const &in) const override
    {
        // Singularity avoider scales with turbine size (outermost chord).
        double avoid = 0.01 * in.chord + in.tip_extra_distance;
        double f_T = (avoid + in.rotor_radius - in.radius) /
                     (in.radius * std::abs(std::sin(in.phi)));
        double arg = std::exp(-0.5 * in.num_blades * f_T);
        return 2.0 / M_PI * std::acos(std::clamp(arg, 0.0, 1.0));
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Classic Prandtl hub-loss factor (F_H).
// ─────────────────────────────────────────────────────────────────────────────
class PrandtlHubLoss final : public ILossModel
{
public:
    double Evaluate(LossModelInput const &in) const override
    {
        double avoid = 0.01 * in.chord;
        double f_H = (avoid + in.radius - in.hub_radius) /
                     (in.hub_radius * std::abs(std::sin(in.phi)));
        double arg = std::exp(-0.5 * in.num_blades * f_H);
        return 2.0 / M_PI * std::acos(std::clamp(arg, 0.0, 1.0));
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Combined tip × hub loss.
// ─────────────────────────────────────────────────────────────────────────────
class CombinedLoss final : public ILossModel
{
public:
    CombinedLoss(ILossModel const *tip, ILossModel const *hub)
        : tip_(tip), hub_(hub) {}

    double Evaluate(LossModelInput const &in) const override
    {
        return tip_->Evaluate(in) * hub_->Evaluate(in);
    }

private:
    ILossModel const *tip_;
    ILossModel const *hub_;
};
