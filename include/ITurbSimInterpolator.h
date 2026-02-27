#pragma once
/**
 * @file ITurbSimInterpolator.h
 * @brief Strategy interface for interpolating a TurbSim velocity field.
 *
 * Single Responsibility: knows only how to interpolate a 3-D velocity
 * vector at an arbitrary (y, z) point within one timestep's grid.
 *
 * Open/Closed: bilinear interpolation is the default; nearest-neighbour,
 * bicubic, or any other scheme can be swapped in by implementing this
 * interface.  TurbSimManager does not change.
 *
 * Interface Segregation: consumers of VelocityAt() are not forced to
 * depend on file-reading or storage methods.
 */
#include <cstddef>
#include "MathUtilities.h" // WVPMUtilities::Vec3D
#include "TurbSimGrid.h"
#include "TurbSimVelocityData.h"

class ITurbSimInterpolator
{
public:
    virtual ~ITurbSimInterpolator() = default;

    /**
     * @brief Interpolate a velocity vector at position (y, z) for the given
     *        timestep.
     *
     * @param point     3-D query point (only y and z components are used).
     * @param raw_ts    Raw (padded) timestep index into the velocity data.
     * @param grid      Grid geometry providing axis breakpoints.
     * @param velocity  Velocity data storage.
     * @return          Interpolated 3-D velocity vector [m/s].
     */
    virtual WVPMUtilities::Vec3D<double> Interpolate(
        WVPMUtilities::Vec3D<double> const &point,
        std::size_t raw_ts,
        TurbSimGrid const &grid,
        TurbSimVelocityData const &velocity) const = 0;
};
