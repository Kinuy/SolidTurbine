#pragma once
/**
 * @file IPitchSchedule.h
 * @brief Strategy interface for pitch-angle scheduling.
 *
 * Single Responsibility: maps electrical power → pitch offset.
 * Open/Closed: fixed pitch, tabular schedule, or optimisation-based pitch
 * are all just implementations.
 */

class IPitchSchedule
{
public:
    virtual ~IPitchSchedule() = default;

    /**
     * @brief Return the collective pitch offset [deg] for the given
     *        electrical power output [W].
     */
    virtual double PitchOffset(double p_el) const = 0;

    /**
     * @brief Return the baseline (minimum) pitch angle [deg].
     */
    virtual double BasePitch() const = 0;
};
