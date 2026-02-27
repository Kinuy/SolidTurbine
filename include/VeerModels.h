#pragma once
/**
 * @file VeerModels.h
 * @brief Concrete wind-veer model implementations.
 *
 * Single Responsibility: each class handles exactly one veer formulation.
 * Open/Closed: new formulations are added here without modifying FlowCalculator.
 */
#define _USE_MATH_DEFINES
#include <array>
#include <cmath>
#include "IVeerModel.h"
#include "MathUtilities.h" // WVPMUtilities::SquareMatrix, RotateVec3D

// ─────────────────────────────────────────────────────────────────────────────
// No veer — identity transform.
// ─────────────────────────────────────────────────────────────────────────────
class NoVeer final : public IVeerModel
{
public:
    WVPMUtilities::Vec3D<double> Apply(
        WVPMUtilities::Vec3D<double> const &vel,
        VeerInput const & /*in*/) const override
    {
        return vel;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Linear veer — direction changes linearly with height deviation from hub.
//
// veer_rate [deg/m]:  dθ/dz — positive = clockwise when viewed from upstream.
// The implementation rotates the velocity vector in the horizontal plane (z-axis).
// ─────────────────────────────────────────────────────────────────────────────
class LinearVeerModel final : public IVeerModel
{
public:
    /**
     * @param veer_rate  Wind-direction change per unit height [deg/m].
     *                   Negative = counter-clockwise viewed from upstream.
     * @param rotor_radius  Rotor radius [m] — used to normalise the rate as in
     *                      the original code (rate is per rotor-radius, not per m).
     *                      Pass 1.0 if rate is already in deg/m.
     */
    explicit LinearVeerModel(double veer_rate_deg_per_R, double rotor_radius)
        : veer_rate_rad_per_m_(veer_rate_deg_per_R * M_PI / (180.0 * rotor_radius))
    {
    }

    WVPMUtilities::Vec3D<double> Apply(
        WVPMUtilities::Vec3D<double> const &vel,
        VeerInput const &in) const override
    {
        // Angle at this height relative to hub height
        double veer_angle = -veer_rate_rad_per_m_ * (in.height - in.hub_height);

        // Rotation matrix about z-axis (yaw in horizontal plane)
        double c = std::cos(veer_angle);
        double s = std::sin(veer_angle);

        std::array<double, 9> m = {
            c, -s, 0.0,
            s, c, 0.0,
            0.0, 0.0, 1.0};

        WVPMUtilities::SquareMatrix<double> mat(3);
        for (std::size_t i = 0; i < m.size(); ++i)
            mat[i] = m[i];

        WVPMUtilities::Vec3D<double> rotated = vel;
        WVPMUtilities::RotateVec3D(mat, &rotated);
        return rotated;
    }

private:
    double veer_rate_rad_per_m_;
};
