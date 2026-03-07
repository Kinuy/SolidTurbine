#pragma once
/**
 * @file NoiseConfig.h
 * @brief Method-selection flags for blade section noise calculation.
 *
 * Extracted from SectionNoiseCalculator.h so that ISectionNoiseConfigBuilder
 * and BladeNoiseConfigBuilder can include it without pulling in the full
 * SectionNoiseCalculator.
 */

struct NoiseConfig
{
    int  bl_tripping          = 0; ///< 0=no trip, 1=heavy, 2=light
    int  bl_properties_method = 1; ///< 1=BPM, 2=Xfoil
    int  tbl_noise_method     = 1; ///< 0=none, 1=BPM, 2=TNO
    int  ti_noise_method      = 0; ///< 0=none, 1=Amiet, 2=Guidati, 3=Simplified
    bool compute_bluntness    = true;
    bool compute_laminar      = true;

    /// True if any noise source is enabled.
    bool any_enabled() const
    {
        return tbl_noise_method != 0
            || ti_noise_method  != 0
            || compute_bluntness
            || compute_laminar;
    }
};
