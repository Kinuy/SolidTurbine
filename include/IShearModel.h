#pragma once
/**
 * @file IShearModel.h
 * @brief Strategy interface for atmospheric wind-shear models.
 *
 * Open/Closed Principle: logarithmic, power-law, and diabatic shear models
 * are all independent implementations.  Adding a new shear model (e.g.
 * WRF-profile) means creating a new class — no existing code changes.
 *
 * Interface Segregation: callers only ever need VelocityAt().
 * The FlowCalculator does not care which model is active; it calls the
 * interface.
 */
#include <cstddef>
#include "MathUtilities.h" // WVPMUtilities::Vec3D<double>

/**
 * @brief Inputs the shear model needs from the flow state.
 */
struct ShearInput
{
    double height;     ///< height of the blade section above ground [m]
    double hub_height; ///< rotor hub height [m]
    double v_hub;      ///< hub-height free-stream velocity [m/s]
};

/**
 * @brief Abstract shear model.
 *
 * Returns the axial (stream-wise) wind velocity at the given height.
 */
class IShearModel
{
public:
    virtual ~IShearModel() = default;

    /**
     * @brief Compute the axial velocity [m/s] for the given height.
     * @param in  Shear input data.
     * @return    Axial free-stream velocity at the blade section height.
     */
    virtual double VelocityAt(ShearInput const &in) const = 0;
};
