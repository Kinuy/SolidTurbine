#pragma once
/**
 * @file SectionNoiseCalculator.h
 * @brief Orchestrates per-section noise calculation for a full blade.
 *
 * Loops over sections, translates SectionNoiseInput → bladenoise::ProjectConfig,
 * calls bladenoise::noise::NoiseCalculator, and maps results back to
 * SectionNoiseResult — all without exposing bladenoise types to callers.
 *
 * SOLID:
 *  S – responsible only for the loop + config translation + result mapping;
 *      the actual physics are in bladenoise::NoiseCalculator.
 *  O – noise method selection is driven by NoiseConfig (data object), not
 *      by if-chains here.
 *  D – depends on ISectionNoiseAdapter abstraction (not BEMSectionNoiseAdapter).
 */
#include "SectionNoiseInput.h"
#include "SectionNoiseResult.h"
#include "ISectionNoiseAdapter.h"
#include "ISectionNoiseConfigBuilder.h"
#include "ISimulationConfig.h"
#include "BEMPostprocessor.h"
#include <memory>
#include <string>
#include <vector>

class TurbineGeometry;

// ─────────────────────────────────────────────────────────────────────────────
/// Noise method selection — built once from ISimulationConfig, passed around.
#include "NoiseConfig.h"

// ─────────────────────────────────────────────────────────────────────────────
class SectionNoiseCalculator
{
public:
    /**
     * @param noise_config   Method-selection flags.
     * @param adapter        Maps BEM data → SectionNoiseInput per section.
     * @param config_builder Translates SectionNoiseInput → noise result
     *                       (defaults to BladeNoiseConfigBuilder).
     */
    SectionNoiseCalculator(
        NoiseConfig const                          &noise_config,
        std::shared_ptr<ISectionNoiseAdapter>       adapter,
        std::shared_ptr<ISectionNoiseConfigBuilder> config_builder = nullptr);

    /**
     * @brief Compute noise for all sections of one operating point.
     *
     * @param pp          Postprocessed BEM result (alpha, cl, cd, element_length, …).
     * @param turbine     Blade geometry.
     * @param sim_config  Physics constants.
     * @param vinf        Wind speed [m/s].
     * @param local_vel   Local flow velocity per section [m/s]  (from NingSolver).
     * @param local_mach  Local Mach number per section [-].
     * @param local_re    Local Reynolds number per section [-].
     * @return            BladeNoiseResult with one entry per section.
     */
    BladeNoiseResult Calculate(
        BEMPostprocessResult const        &pp,
        TurbineGeometry const             *turbine,
        ISimulationConfig const           &sim_config,
        double                             vinf,
        std::vector<double> const         &local_vel,
        std::vector<double> const         &local_mach,
        std::vector<double> const         &local_re) const;

    std::string get_error() const { return error_; }

private:
    NoiseConfig                                        noise_config_;
    std::shared_ptr<ISectionNoiseAdapter>              adapter_;
    std::shared_ptr<ISectionNoiseConfigBuilder>        config_builder_;
    mutable std::string                                error_;


};
