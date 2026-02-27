#pragma once
/**
 * @file TurbSimVelocityData.h
 * @brief Storage for all TurbSim velocity timesteps.
 *
 * Single Responsibility: stores and retrieves 3-D velocity grids indexed
 * by time step.  No file I/O, no interpolation, no grid geometry.
 *
 * Interface Segregation: read-only consumers (interpolators) call
 * TimestepBegin(); writers (the file reader) call AddTimestep().
 */
#include <cstddef>
#include <stdexcept>
#include <vector>
#include "MathUtilities.h" // WVPMUtilities::Vec3D<double>

/// One time-step's worth of velocity vectors, one per grid point.
using VelocityTimestep = std::vector<WVPMUtilities::Vec3D<double>>;

class TurbSimVelocityData
{
public:
    TurbSimVelocityData() = default;

    // ── Writing ───────────────────────────────────────────────────────────────

    /**
     * @brief Append one timestep of velocity data.
     * @param vt  Flat grid of Vec3D, row-major (z-outer, y-inner).
     */
    void AddTimestep(VelocityTimestep vt)
    {
        timesteps_.push_back(std::move(vt));
    }

    // ── Reading ───────────────────────────────────────────────────────────────

    std::size_t num_timesteps() const { return timesteps_.size(); }

    /**
     * @brief Return the velocity grid for a given time index.
     * @throws std::out_of_range if ts >= num_timesteps().
     */
    VelocityTimestep const &Timestep(std::size_t ts) const
    {
        if (ts >= timesteps_.size())
            throw std::out_of_range("TurbSimVelocityData: timestep index out of range");
        return timesteps_[ts];
    }

    /**
     * @brief Iterator to the first grid point of a given timestep.
     * Useful for the bilinear interpolator which walks the flat array.
     */
    VelocityTimestep::const_iterator TimestepBegin(std::size_t ts) const
    {
        return Timestep(ts).begin();
    }

private:
    std::vector<VelocityTimestep> timesteps_;
};
