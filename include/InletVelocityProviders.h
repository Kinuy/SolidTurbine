#pragma once
/**
 * @file InletVelocityProviders.h
 * @brief Concrete implementations of IInletVelocityProvider.
 *
 * Open/Closed: the FlowCalculator uses IInletVelocityProvider; callers choose
 * which provider to inject.  Adding a new source (e.g. CFD wake, LiDAR data)
 * never modifies FlowCalculator.
 */
#include <stdexcept>
#include "IInletVelocityProvider.h"
#include "TurbSimManager.h" // TurbSimManager — existing type, unchanged

// ─────────────────────────────────────────────────────────────────────────────
// Uniform inflow — constant v_inf in the axial direction at all sections.
// This is the default case (no TurbSim).
// ─────────────────────────────────────────────────────────────────────────────
class UniformInletProvider final : public IInletVelocityProvider
{
public:
    explicit UniformInletProvider(double v_inf)
        : v_inf_(v_inf)
    {
        if (v_inf_ < 0.0)
            throw std::invalid_argument("UniformInletProvider: v_inf must be >= 0");
    }

    WVPMUtilities::Vec3D<double> VelocityAt(
        WVPMUtilities::Vec3D<double> const & /*position*/,
        std::size_t /*section_index*/) const override
    {
        // Pure axial inflow; y and z components are zero (shear/veer add later).
        return WVPMUtilities::Vec3D<double>(v_inf_, 0.0, 0.0);
    }

    double HubVelocity() const override { return v_inf_; }

private:
    double v_inf_;
};

// ─────────────────────────────────────────────────────────────────────────────
// TurbSim time-series inlet — reads wind field from a TurbSimManager.
// Replaces the UseTurbsim() method that was embedded in FlowCalculator.
// ─────────────────────────────────────────────────────────────────────────────
class TurbSimInletProvider final : public IInletVelocityProvider
{
public:
    /**
     * @param tsm        Non-owning pointer to a TurbSimManager (must outlive this).
     * @param iteration  Time step index into the TurbSim time series.
     */
    TurbSimInletProvider(TurbSimManager const *tsm, unsigned iteration)
        : tsm_(tsm), iteration_(iteration)
    {
        if (!tsm_)
            throw std::invalid_argument("TurbSimInletProvider: tsm must be non-null");
    }

    WVPMUtilities::Vec3D<double> VelocityAt(
        WVPMUtilities::Vec3D<double> const &position,
        std::size_t /*section_index*/) const override
    {
        return tsm_->VelocityAt(position, iteration_);
    }

    double HubVelocity() const override
    {
        return tsm_->hub_velocity();
    }

private:
    TurbSimManager const *tsm_;
    unsigned iteration_;
};
