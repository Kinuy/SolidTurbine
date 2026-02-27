/**
 * @file BilinearTurbSimInterpolator.cpp
 * @brief Implementation of BilinearTurbSimInterpolator.
 *
 * Preserves the original "find bounding box first" optimisation that
 * reduced execution time to ~40% of the naïve implementation.
 *
 * Interpolation is performed using MathUtility::basicLinearInterpolation,
 * which is the same primitive that MathUtility::biLinearInterpolation
 * delegates to internally.  Since we have already located the bounding cell
 * (4 corner values), we apply two z-direction lerps followed by one y-direction
 * lerp — identical algebra, zero vector allocations.
 *
 *   Corner layout (z-outer, y-inner storage, matching TurbSim flat arrays):
 *
 *       z[z_idx+1]   v_lo_zhi ---- v_hi_zhi
 *                       |               |
 *       z[z_idx  ]   v_lo_zlo ---- v_hi_zlo
 *                       |               |
 *                    y[y_idx]      y[y_idx+1]
 */
#include "BilinearTurbSimInterpolator.h"

#include <array>
#include <functional>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// Interpolate — public entry point
// ─────────────────────────────────────────────────────────────────────────────
WVPMUtilities::Vec3D<double>
BilinearTurbSimInterpolator::Interpolate(
    WVPMUtilities::Vec3D<double> const &point,
    std::size_t raw_ts,
    TurbSimGrid const &grid,
    TurbSimVelocityData const &velocity) const
{
    // ── Step 1: find bounding grid cell ──────────────────────────────────────
    // Key optimisation: identify the single 2×2 cell that brackets the query
    // point, then work only with its 4 corners — no large vector allocation.

    std::vector<double> const y_axis = grid.axis_y();
    std::vector<double> const z_axis = grid.axis_z();
    std::size_t const num_y = grid.num_y();

    std::size_t const y_idx = FindLowerIndex(y_axis, point.y());
    std::size_t const z_idx = FindLowerIndex(z_axis, point.z());

    // ── Step 2: read the 4 bounding cell corners from the flat grid ──────────
    // Flat index: row z, column y  →  z * num_y + y
    auto vel_begin = velocity.TimestepBegin(raw_ts);

    auto const &v_lo_zlo = *(vel_begin + z_idx * num_y + y_idx);
    auto const &v_lo_zhi = *(vel_begin + (z_idx + 1) * num_y + y_idx);
    auto const &v_hi_zlo = *(vel_begin + z_idx * num_y + y_idx + 1);
    auto const &v_hi_zhi = *(vel_begin + (z_idx + 1) * num_y + y_idx + 1);

    // Axis coordinates of the bounding cell
    double const y_lo = y_axis[y_idx];
    double const y_hi = y_axis[y_idx + 1];
    double const z_lo = z_axis[z_idx];
    double const z_hi = z_axis[z_idx + 1];

    // ── Step 3: bilinear interpolation per component ──────────────────────────
    // Uses MathUtility::basicLinearInterpolation (the same primitive that
    // MathUtility::biLinearInterpolation delegates to):
    //   Step 3a — lerp in z at the left  column (y = y_lo)  → v_left
    //   Step 3b — lerp in z at the right column (y = y_hi)  → v_right
    //   Step 3c — lerp in y between v_left and v_right       → result

    auto biLerp = [&](double q_lo_zlo, double q_lo_zhi,
                      double q_hi_zlo, double q_hi_zhi) -> double
    {
        double v_left = MathUtility::basicLinearInterpolation(
            point.z(), z_lo, z_hi, q_lo_zlo, q_lo_zhi);
        double v_right = MathUtility::basicLinearInterpolation(
            point.z(), z_lo, z_hi, q_hi_zlo, q_hi_zhi);
        return MathUtility::basicLinearInterpolation(
            point.y(), y_lo, y_hi, v_left, v_right);
    };

    using V = WVPMUtilities::Vec3D<double>;
    return V(
        biLerp(v_lo_zlo.x(), v_lo_zhi.x(), v_hi_zlo.x(), v_hi_zhi.x()),
        biLerp(v_lo_zlo.y(), v_lo_zhi.y(), v_hi_zlo.y(), v_hi_zhi.y()),
        biLerp(v_lo_zlo.z(), v_lo_zhi.z(), v_hi_zlo.z(), v_hi_zhi.z()));
}

// ─────────────────────────────────────────────────────────────────────────────
// FindLowerIndex
//
// Returns the index i such that axis[i] <= value < axis[i+1].
// Clamps to [0, axis.size()-2] for points outside the grid.
// ─────────────────────────────────────────────────────────────────────────────
std::size_t BilinearTurbSimInterpolator::FindLowerIndex(
    std::vector<double> const &axis,
    double value)
{
    if (axis.size() < 2)
        throw std::invalid_argument("BilinearTurbSimInterpolator: axis must have >= 2 points");

    // Clamp above-maximum points to the last cell.
    if (value >= axis.back())
        return axis.size() - 2;

    for (std::size_t i = 0; i < axis.size() - 1; ++i)
    {
        if (value <= axis[i + 1])
            return i;
    }

    return 0; // value <= axis.front() — clamp to first cell
}
