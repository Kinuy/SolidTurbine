#pragma once
/**
 * @file StandardPitchSchedule.h
 * @brief Piecewise-linear pitch-offset schedule (port of opermot::get_gamma_offset).
 *
 * Single Responsibility: owns only the P_el → gamma_offset mapping.
 */
#include <vector>
#include <stdexcept>
#include "IPitchSchedule.h"

class StandardPitchSchedule final : public IPitchSchedule
{
public:
    /**
     * @param base_pitch   Minimum / optimal pitch angle [deg].
     * @param pmin_breaks  Power breakpoints [W].
     * @param delta_alfa   Pitch-rate coefficients matching each breakpoint.
     */
    StandardPitchSchedule(double base_pitch,
                          std::vector<double> pmin_breaks,
                          std::vector<double> delta_alfa);

    double BasePitch() const override;

    double PitchOffset(double p_el) const override;

private:
    double base_pitch_;
    std::vector<double> pmin_;
    std::vector<double> delta_;
};
