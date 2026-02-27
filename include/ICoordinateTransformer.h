#pragma once
/**
 * @file ICoordinateTransformer.h
 * @brief Strategy interface for global-to-local coordinate transforms.
 *
 * Single Responsibility: owns only the coordinate rotation from global
 * (hub-fixed) to local (blade-section) frame.
 *
 * Interface Segregation: callers need only ToLocal().  The rotation matrix
 * computation is hidden inside the implementation.
 */
#include "MathUtilities.h"

class ICoordinateTransformer
{
public:
    virtual ~ICoordinateTransformer() = default;

    /**
     * @brief Convert a velocity vector from global to blade-local coordinates.
     * @param global_vel  Velocity in the global (hub-fixed) frame [m/s].
     * @return            Velocity in the blade-section local frame [m/s].
     */
    virtual WVPMUtilities::Vec3D<double> ToLocal(
        WVPMUtilities::Vec3D<double> const &global_vel) const = 0;
};
