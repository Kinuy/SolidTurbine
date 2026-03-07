#pragma once
/**
 * @file BladeNoiseConfigBuilder.h
 * @brief Concrete ISectionNoiseConfigBuilder backed by the bladenoise library.
 *
 * This is the ONLY header in the SolidTurbine project that names any
 * bladenoise type.  All other SolidTurbine translation units depend on
 * ISectionNoiseConfigBuilder or SectionNoiseInput/Result only.
 *
 * When bl_method == XFOIL the builder requires actual airfoil coordinate
 * data.  Pass a pre-converted bladenoise::io::AirfoilData via the
 * optional airfoil_data pointer in SectionNoiseInput (airfoil_file field
 * identifies the section; the caller supplies the coordinate data).
 * Use AirfoilDataAdapter::Convert() to produce it from AirfoilGeometryData.
 *
 * SOLID:
 *  S – fills bladenoise::ProjectConfig and delegates to NoiseCalculator.
 *  L – fully satisfies ISectionNoiseConfigBuilder.
 *  D – SectionNoiseCalculator never includes this header.
 */
#include "ISectionNoiseConfigBuilder.h"
#include "bladenoise/io/IOTypes.h"
#include <memory>

class BladeNoiseConfigBuilder final : public ISectionNoiseConfigBuilder
{
public:
    bool Build(
        SectionNoiseInput const &inp,
        NoiseConfig       const &noise_cfg,
        SectionNoiseResult      &result) const override;
};
