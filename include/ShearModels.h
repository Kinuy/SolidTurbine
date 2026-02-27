#pragma once
/**
 * @file ShearModels.h
 * @brief Concrete atmospheric shear model implementations.
 *
 * Each class is responsible for exactly one wind-profile formula (SRP).
 * New models are added here without touching FlowCalculator (OCP).
 *
 * All models derive from IShearModel and implement VelocityAt().
 */
#define _USE_MATH_DEFINES
#include <cmath>
#include <stdexcept>
#include "IShearModel.h"

// ─────────────────────────────────────────────────────────────────────────────
// Logarithmic shear  v(z) = v_ref * ln(z/z0) / ln(z_ref/z0)
// ─────────────────────────────────────────────────────────────────────────────
class LogShearModel final : public IShearModel
{
public:
    /**
     * @param surface_roughness  Aerodynamic roughness length z0 [m].
     * @param ref_vel            Reference velocity [m/s].
     *                           When use_ref_hv is false, v_hub is used instead.
     * @param ref_height         Reference height [m].
     *                           When use_ref_hv is false, hub_height is used.
     * @param use_ref_hv         True  → use ref_vel / ref_height.
     *                           False → use v_hub / hub_height from ShearInput.
     */
    LogShearModel(double surface_roughness,
                  double ref_vel = 0.0,
                  double ref_height = 0.0,
                  bool use_ref_hv = false)
        : z0_(surface_roughness), ref_vel_(ref_vel), ref_height_(ref_height), use_ref_hv_(use_ref_hv)
    {
        if (z0_ <= 0.0)
            throw std::invalid_argument("LogShearModel: surface_roughness must be > 0");
    }

    double VelocityAt(ShearInput const &in) const override
    {
        double top = std::log(in.height / z0_);
        double v_ref = use_ref_hv_ ? ref_vel_ : in.v_hub;
        double z_ref = use_ref_hv_ ? ref_height_ : in.hub_height;
        double bottom = std::log(z_ref / z0_);
        return v_ref * top / bottom;
    }

private:
    double z0_;
    double ref_vel_;
    double ref_height_;
    bool use_ref_hv_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Power-law (exponential) shear  v(z) = v_ref * (z / z_ref)^α
// ─────────────────────────────────────────────────────────────────────────────
class PowerLawShearModel final : public IShearModel
{
public:
    /**
     * @param exponent    Wind shear exponent α (typically 0.1–0.3 offshore/onshore).
     * @param ref_vel     Reference velocity [m/s].
     * @param ref_height  Reference height [m].
     * @param use_ref_hv  True  → use ref_vel / ref_height.
     *                    False → use v_hub / hub_height from ShearInput.
     */
    PowerLawShearModel(double exponent,
                       double ref_vel = 0.0,
                       double ref_height = 0.0,
                       bool use_ref_hv = false)
        : alpha_(exponent), ref_vel_(ref_vel), ref_height_(ref_height), use_ref_hv_(use_ref_hv)
    {
    }

    double VelocityAt(ShearInput const &in) const override
    {
        double v_ref = use_ref_hv_ ? ref_vel_ : in.v_hub;
        double z_ref = use_ref_hv_ ? ref_height_ : in.hub_height;
        return v_ref * std::pow(in.height / z_ref, alpha_);
    }

private:
    double alpha_;
    double ref_vel_;
    double ref_height_;
    bool use_ref_hv_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Diabatic (Monin-Obukhov) shear  v(z) = (u*/κ) * [ln(z/z0) - ψ(z/L)]
// ─────────────────────────────────────────────────────────────────────────────
class DiabaticShearModel final : public IShearModel
{
public:
    /**
     * @param surface_roughness  Aerodynamic roughness length z0 [m].
     * @param obukhov_length     Monin-Obukhov length L [m].
     *                           L < 0 → unstable, L > 0 → stable, L = 0 → neutral.
     */
    DiabaticShearModel(double surface_roughness,
                       double obukhov_length)
        : z0_(surface_roughness), L_(obukhov_length)
    {
        if (z0_ <= 0.0)
            throw std::invalid_argument("DiabaticShearModel: surface_roughness must be > 0");
    }

    double VelocityAt(ShearInput const &in) const override
    {
        double corr_hub = StabilityCorrection(in.hub_height, L_);
        double vstar = in.v_hub * kappa_ / (std::log(in.hub_height / z0_) - corr_hub);

        double corr_sec = StabilityCorrection(in.height, L_);
        return vstar / kappa_ * (std::log(in.height / z0_) - corr_sec);
    }

private:
    double z0_;
    double L_;
    static constexpr double kappa_ = 0.4; ///< von Kármán constant

    /**
     * @brief Stability correction function ψ(h/L).
     *
     * Unstable (L<0): Paulson (1970) / Högström (1988) formulation.
     * Stable   (L>0): Linear  −5 h/L.
     * Neutral  (L=0): ψ = 0.
     */
    static double StabilityCorrection(double h, double L)
    {
        if (L < 0.0)
        {
            double x = std::pow(1.0 - 16.0 * h / L, 0.25);
            return 2.0 * std::log((1.0 + x) / 2.0) + std::log((1.0 + x * x) / 2.0) - 2.0 * std::atan(x) + M_PI / 2.0;
        }
        if (L > 0.0)
        {
            return -5.0 * h / L;
        }
        return 0.0; // neutral
    }
};
