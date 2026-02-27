#pragma once
/**
 * @file TurbSimTimingInfo.h
 * @brief Value type that captures the timing and padding metadata of a
 *        TurbSim binary file.
 *
 * Single Responsibility: owns only the arithmetic around padding timesteps
 * and usable iteration count.  Previously this was scatter-computed inside
 * TurbSimManager::Read() and stored in member variables.
 *
 * Moving it here makes it testable in isolation and documents the intent.
 */
#include <cmath>
#include <stdexcept>

/**
 * @brief Immutable timing descriptor computed from .wnd file parameters.
 */
class TurbSimTimingInfo
{
public:
    /**
     * @param total_timesteps        Total number of velocity grids in the file
     *                               (nt * 2 in the original code).
     * @param hub_velocity           Mean hub-height wind speed [m/s].
     * @param longitudinal_grid_res  Longitudinal grid resolution Δx [m].
     * @param grid_width_y           Physical width of the grid in y [m] = (num_y-1)*spacing_y.
     */
    TurbSimTimingInfo(std::size_t total_timesteps,
                      double hub_velocity,
                      double longitudinal_grid_res,
                      double grid_width_y)
    {
        if (hub_velocity <= 0.0)
            throw std::invalid_argument("TurbSimTimingInfo: hub_velocity must be > 0");

        timestep_ = longitudinal_grid_res / hub_velocity;
        double pad_time = grid_width_y / hub_velocity;
        padding_ = static_cast<unsigned>(std::ceil(pad_time / 2.0 / timestep_));
        usable_iters_ = static_cast<unsigned>(total_timesteps) - 2u * padding_;
        usable_time_ = usable_iters_ * timestep_;
    }

    // ── Accessors ─────────────────────────────────────────────────────────────

    /// Length of each time step [s].
    double timestep() const { return timestep_; }

    /// Number of padding timesteps on each end of the dataset.
    unsigned padding() const { return padding_; }

    /// Number of usable (non-padded) iterations.
    unsigned usable_iterations() const { return usable_iters_; }

    /// Total duration of usable wind data [s].
    double usable_time() const { return usable_time_; }

    /**
     * @brief Convert a user-facing iteration index to the raw storage index.
     *
     * User iteration 0 → raw index = padding.
     */
    std::size_t RawIndex(unsigned user_iteration) const
    {
        return static_cast<std::size_t>(user_iteration) + padding_;
    }

private:
    double timestep_{0.0};
    unsigned padding_{0};
    unsigned usable_iters_{0};
    double usable_time_{0.0};
};
