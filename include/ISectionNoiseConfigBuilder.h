#pragma once
/**
 * @file ISectionNoiseConfigBuilder.h
 * @brief Interface: translates SectionNoiseInput + NoiseConfig into a noise
 *        result for one blade section.
 *
 * This is the single seam between the SolidTurbine domain and the bladenoise
 * library.  Everything bladenoise-specific (ProjectConfig, Types, enums,
 * NoiseCalculator) lives exclusively behind this interface.
 *
 * SOLID:
 *  S – responsible only for the per-section translate-and-run step.
 *  O – alternative noise backends (OpenFAST AeroAcoustics, TNO, …) implement
 *      this interface without touching SectionNoiseCalculator.
 *  D – SectionNoiseCalculator depends on this abstraction; the concrete
 *      builder is the only TU that #includes any bladenoise header.
 */
#include "SectionNoiseInput.h"
#include "SectionNoiseResult.h"
#include "NoiseConfig.h"

class ISectionNoiseConfigBuilder
{
public:
    virtual ~ISectionNoiseConfigBuilder() = default;

    /**
     * @brief Translate one section's input into a noise result.
     *
     * @param inp        Per-section geometry, flow state and observer data.
     * @param noise_cfg  Method-selection flags (BL method, TBL, TI, …).
     * @param result     Output: filled on success, zeroed on failure.
     * @return           true if the calculation converged.
     */
    virtual bool Build(
        SectionNoiseInput  const &inp,
        NoiseConfig        const &noise_cfg,
        SectionNoiseResult       &result) const = 0;
};
