/**
 * @file StandardPitchSchedule.cpp
 * @brief Implementation of StandardPitchSchedule.
 *
 * Piecewise-linear pitch-offset scheduling as a function of electrical power.
 * Extracted from opermot::get_gamma_offset().
 */
#include "StandardPitchSchedule.h"

#include <stdexcept>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────
StandardPitchSchedule::StandardPitchSchedule(double base_pitch,
                                             std::vector<double> pmin_breaks,
                                             std::vector<double> delta_alfa)
    : base_pitch_(base_pitch), pmin_(std::move(pmin_breaks)), delta_(std::move(delta_alfa))
{
    if (pmin_.size() != delta_.size())
        throw std::invalid_argument(
            "StandardPitchSchedule: pmin and delta_alfa vectors must be the same length");
    if (pmin_.empty())
        throw std::invalid_argument(
            "StandardPitchSchedule: at least one breakpoint is required");
}

// ─────────────────────────────────────────────────────────────────────────────
// PitchOffset — compute collective pitch offset [deg] for p_el [W]
//
// The schedule is defined as a piecewise-linear ramp:
//   For each segment i: offset += dP_i / 100 000 * delta_i
// where dP_i is the power increment within that segment.
// The denominator 100 000 W = 100 kW normalises the delta coefficients.
// ─────────────────────────────────────────────────────────────────────────────
double StandardPitchSchedule::PitchOffset(double p_el) const
{
    // Below the first breakpoint: no offset
    if (p_el <= pmin_.front())
        return 0.0;

    double offset = 0.0;
    for (std::size_t i = 0; i + 1 < pmin_.size(); ++i)
    {
        double p_lo = pmin_[i];
        double p_hi = pmin_[i + 1];

        if (p_el >= p_hi)
        {
            // Full segment contribution
            offset += (p_hi - p_lo) / 100'000.0 * delta_[i];
        }
        else
        {
            // Partial last segment
            offset += (p_el - p_lo) / 100'000.0 * delta_[i];
            break;
        }
    }

    // Handle power above all breakpoints using the last delta coefficient
    if (p_el > pmin_.back())
    {
        offset += (p_el - pmin_.back()) / 100'000.0 * delta_.back();
    }

    return offset;
}

// ─────────────────────────────────────────────────────────────────────────────
// BasePitch
// ─────────────────────────────────────────────────────────────────────────────
double StandardPitchSchedule::BasePitch() const
{
    return base_pitch_;
}
