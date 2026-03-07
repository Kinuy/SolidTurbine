/**
 * @file SectionNoiseCalculator.cpp
 * @brief Orchestrates per-section noise calculation for a full blade.
 *
 * ── Dependency isolation ─────────────────────────────────────────────────────
 * This file includes NO bladenoise headers.  All library-specific work is
 * delegated to ISectionNoiseConfigBuilder (typically BladeNoiseConfigBuilder).
 * This is the key change from the previous version:
 *
 *   Before: SectionNoiseCalculator.cpp → bladenoise/core/ProjectConfig.h
 *                                      → bladenoise/core/Types.h
 *                                      → bladenoise/noise/NoiseCalculator.h
 *                                      → bladenoise/io/IOTypes.h
 *
 *   After:  SectionNoiseCalculator.cpp → ISectionNoiseConfigBuilder  (interface)
 *           BladeNoiseConfigBuilder.cpp → bladenoise/ (single seam)
 */
#include "SectionNoiseCalculator.h"
#include "ISectionNoiseConfigBuilder.h"
#include "BladeNoiseConfigBuilder.h"   // default implementation
#include "TurbineGeometry.h"

#include <cmath>
#include <iostream>
#include <numbers>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
SectionNoiseCalculator::SectionNoiseCalculator(
    NoiseConfig const                          &noise_config,
    std::shared_ptr<ISectionNoiseAdapter>       adapter,
    std::shared_ptr<ISectionNoiseConfigBuilder> config_builder)
    : noise_config_(noise_config)
    , adapter_(std::move(adapter))
    , config_builder_(config_builder
                        ? std::move(config_builder)
                        : std::make_shared<BladeNoiseConfigBuilder>())
{}

// ─────────────────────────────────────────────────────────────────────────────
// Calculate
// ─────────────────────────────────────────────────────────────────────────────
BladeNoiseResult SectionNoiseCalculator::Calculate(
    BEMPostprocessResult const  &pp,
    TurbineGeometry const       *turbine,
    ISimulationConfig const     &sim_config,
    double                       vinf,
    std::vector<double> const   &local_vel,
    std::vector<double> const   &local_mach,
    std::vector<double> const   &local_re) const
{
    BladeNoiseResult blade_result;
    blade_result.vinf = vinf;

    if (!turbine || !adapter_ || !config_builder_)
    {
        error_ = "SectionNoiseCalculator: null turbine, adapter or config_builder";
        return blade_result;
    }

    // ── Build per-section inputs from BEM postprocessor data ──────────────────
    auto inputs = adapter_->Build(pp, turbine, sim_config, vinf);
    if (inputs.empty())
    {
        error_ = "SectionNoiseCalculator: adapter returned no inputs";
        return blade_result;
    }

    // ── Fill per-section velocities from stored BEM fields ────────────────────
    const std::size_t n = inputs.size();
    for (std::size_t i = 0; i < n; ++i)
    {
        if (i < local_vel.size())  inputs[i].velocity = local_vel[i];
        if (i < local_mach.size()) inputs[i].mach     = local_mach[i];
        if (i < local_re.size())   inputs[i].reynolds = local_re[i];
    }

    // ── Frequency vector — derived from bladenoise constants, but only the
    //    count matters here; actual values come back in SectionNoiseSpectrum.
    //    We query from the first successful result below; pre-allocate 34 bands.
    blade_result.sections.reserve(n);
    int n_failed = 0;

    for (std::size_t i = 0; i < n; ++i)
    {
        SectionNoiseInput const &inp = inputs[i];

        // ── Guard: skip sections with no flow ─────────────────────────────────
        if (inp.velocity <= 0.0)
        {
            SectionNoiseResult sr;
            sr.section_index = i;
            sr.radius        = turbine->radius(i);
            sr.chord         = inp.chord;
            sr.span          = inp.span;
            sr.converged     = false;
            blade_result.sections.push_back(sr);
            continue;
        }

        // ── Delegate translate-and-run to ISectionNoiseConfigBuilder ──────────
        SectionNoiseResult sr;
        sr.section_index = i;
        sr.radius        = turbine->radius(i);
        sr.chord         = inp.chord;
        sr.span          = inp.span;
        sr.velocity      = inp.velocity;
        sr.mach          = inp.mach;
        sr.reynolds      = inp.reynolds;
        sr.alpha_deg     = inp.alpha_deg;

        sr.converged = config_builder_->Build(inp, noise_config_, sr);

        if (!sr.converged)
            ++n_failed;

        blade_result.sections.push_back(std::move(sr));
    }

    if (n_failed > 0)
        std::cout << "  SectionNoiseCalculator: " << (n - n_failed) << "/"
                  << n << " sections converged\n";

    // ── Populate frequency list from first converged section ──────────────────
    for (auto const &sec : blade_result.sections)
        if (sec.converged && !sec.total.spl.empty())
        {
            // Rebuild standard 1/3-octave centres — they are constant
            // (34 bands, 10 Hz – 20 kHz); we can't easily query from the
            // bladenoise constants here without including bladenoise headers,
            // so we infer the count and reconstruct the same sequence used
            // by bladenoise::constants::THIRD_OCTAVE_BANDS.
            static const std::vector<double> kBands = {
                10,12.5,16,20,25,31.5,40,50,63,80,
                100,125,160,200,250,315,400,500,630,800,
                1000,1250,1600,2000,2500,3150,
                4000,5000,6300,8000,10000,12500,16000,20000
            };
            blade_result.frequencies = kBands;
            break;
        }

    return blade_result;
}
