#pragma once
/**
 * @file IVeerModel.h
 * @brief Strategy interface for wind-veer models.
 *
 * Single Responsibility: the veer model owns only the rotation of a
 * velocity vector due to directional change with height.
 *
 * Open/Closed: alternative veer models (e.g. height-power-law veer)
 * can be plugged in without touching FlowCalculator.
 */
#include "MathUtilities.h" // WVPMUtilities::Vec3D<double>

/**
 * @brief Inputs the veer model needs.
 */
struct VeerInput
{
    double height;     ///< height of the blade section above ground [m]
    double hub_height; ///< rotor hub height [m]
};

/**
 * @brief Abstract veer model.
 *
 * Takes a velocity vector in global coordinates and returns the
 * veer-rotated version.
 */
class IVeerModel
{
public:
    virtual ~IVeerModel() = default;

    /**
     * @brief Apply veer rotation to a global-frame velocity vector.
     *
     * @param vel  Input velocity vector [m/s] in global coordinates.
     * @param in   Height-related context.
     * @return     Veer-rotated velocity vector [m/s].
     */
    virtual WVPMUtilities::Vec3D<double> Apply(
        WVPMUtilities::Vec3D<double> const &vel,
        VeerInput const &in) const = 0;
};
