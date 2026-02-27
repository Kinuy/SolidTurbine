#pragma once
/**
 * @file BilinearTurbSimInterpolator.h
 * @brief Bilinear interpolation of TurbSim velocity grids.
 *
 * Single Responsibility: owns only the bilinear interpolation algorithm
 * for a 2-D (y, z) velocity grid.  No file I/O, no timing arithmetic.
 *
 * Uses MathUtility::basicLinearInterpolation as the underlying primitive —
 * the same function that MathUtility::biLinearInterpolation delegates to.
 * Applying it directly avoids vector allocation overhead; the bounding cell
 * is already known from the grid-box search.
 *
 * Preserves the original "find bounding box first" optimisation that
 * reduced execution time to ~40% of the naïve implementation.
 */
#include <cstddef>
#include <vector>
#include "ITurbSimInterpolator.h"
#include "MathUtility.h" // MathUtility::basicLinearInterpolation

class BilinearTurbSimInterpolator final : public ITurbSimInterpolator
{
public:
    /**
     * @brief Interpolate a 3-D velocity vector using bilinear interpolation.
     *
     * Finds the bounding grid cell first (O(n) scan, fast due to small grid
     * sizes), then applies two z-direction lerps and one y-direction lerp
     * independently for each velocity component (u, v, w).
     *
     * Points outside the grid are clamped to the nearest boundary cell.
     *
     * @param point     Query position; only y() and z() are used.
     * @param raw_ts    Raw (padded) time-step index into the velocity store.
     * @param grid      Grid geometry providing y and z axis breakpoints.
     * @param velocity  Velocity data storage.
     */
    WVPMUtilities::Vec3D<double> Interpolate(
        WVPMUtilities::Vec3D<double> const &point,
        std::size_t raw_ts,
        TurbSimGrid const &grid,
        TurbSimVelocityData const &velocity) const override;

private:
    /// Return the lower bounding index i such that axis[i] <= value < axis[i+1].
    /// Clamps to [0, axis.size()-2] for out-of-range values.
    static std::size_t FindLowerIndex(std::vector<double> const &axis,
                                      double value);
};
