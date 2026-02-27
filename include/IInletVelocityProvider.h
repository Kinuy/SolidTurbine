#pragma once
/**
 * @file IInletVelocityProvider.h
 * @brief Strategy interface for the source of global inlet velocities.
 *
 * Single Responsibility: determines the *unmodified* global velocity at each
 * blade section before atmospheric modifiers (shear, veer) are applied.
 *
 * Open/Closed: a uniform inflow, a TurbSim time-series, or a CFD wake
 * field can all implement this interface.  FlowCalculator does not change
 * when the inlet source changes.
 *
 * Dependency Inversion: FlowCalculator depends on this abstraction, not on
 * TurbSimManager directly.
 */
#include <cstddef>
#include <vector>
#include "MathUtilities.h"

/**
 * @brief Pure interface for inlet velocity providers.
 */
class IInletVelocityProvider
{
public:
    virtual ~IInletVelocityProvider() = default;

    /**
     * @brief Return the inlet velocity at a blade section in global coordinates.
     *
     * @param section_position  3-D global position of the blade section [m].
     * @param section_index     Radial section index (0 = innermost).
     * @return                  Global velocity vector [m/s].
     */
    virtual WVPMUtilities::Vec3D<double> VelocityAt(
        WVPMUtilities::Vec3D<double> const &section_position,
        std::size_t section_index) const = 0;

    /**
     * @brief Hub-height reference velocity [m/s] for shear normalisation.
     */
    virtual double HubVelocity() const = 0;
};
