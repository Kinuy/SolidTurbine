/**
 * @file BladeNoiseConfigBuilder.cpp
 * @brief The single translation unit that #includes bladenoise headers.
 *
 * ── Dependency isolation ─────────────────────────────────────────────────────
 * Only this file sees:
 *   bladenoise/core/ProjectConfig.h   – config struct filled from SectionNoiseInput
 *   bladenoise/core/Types.h           – TripConfig, BoundaryLayerMethod, …
 *   bladenoise/noise/NoiseCalculator.h – the physics engine
 *   bladenoise/io/IOTypes.h           – io::AirfoilData (empty for BPM path)
 *
 * Everything else in SolidTurbine depends only on ISectionNoiseConfigBuilder.
 */
#include "BladeNoiseConfigBuilder.h"

// ── bladenoise includes — isolated here intentionally ─────────────────────────
#include "bladenoise/core/ProjectConfig.h"
#include "bladenoise/core/Types.h"
#include "bladenoise/noise/NoiseCalculator.h"
#include "bladenoise/io/IOTypes.h"

#include <cmath>
#include <iostream>
#include <numbers>

using namespace bladenoise;

// ─────────────────────────────────────────────────────────────────────────────
bool BladeNoiseConfigBuilder::Build(
    SectionNoiseInput const &inp,
    NoiseConfig       const &nc,
    SectionNoiseResult      &result) const
{
    // ── Fill bladenoise::ProjectConfig from SolidTurbine domain types ─────────
    ProjectConfig pcfg;

    // Atmospheric constants
    pcfg.speed_of_sound       = inp.speed_of_sound;
    pcfg.kinematic_viscosity  = inp.kinematic_viscosity;
    pcfg.air_density          = inp.air_density;

    // Blade element geometry
    pcfg.chord               = inp.chord;
    pcfg.span                = inp.span;
    pcfg.freestream_velocity = inp.velocity;
    pcfg.angle_of_attack     = inp.alpha_deg;   // [deg] — ProjectConfig takes deg

    // Trailing edge
    pcfg.trailing_edge_thickness = inp.te_thickness;
    pcfg.trailing_edge_angle     = inp.te_angle_deg;

    // Boundary layer method & trip config
    pcfg.trip_config = static_cast<TripConfig>(nc.bl_tripping);
    pcfg.bl_method   = static_cast<BoundaryLayerMethod>(nc.bl_properties_method);

    // Noise method selection
    pcfg.tbl_method        = static_cast<TBLNoiseMethod>(nc.tbl_noise_method);
    pcfg.ti_method         = static_cast<TINoiseMethod>(nc.ti_noise_method);
    pcfg.compute_bluntness = nc.compute_bluntness;
    pcfg.compute_laminar   = nc.compute_laminar;

    // Turbulent inflow
    pcfg.turbulence_intensity    = inp.turbulence_intensity * 100.0; // fraction → percent
    pcfg.turbulence_length_scale = inp.turbulence_length_scale;

    // Thickness fractions for TI noise
    pcfg.thickness_1_percent  = inp.thickness_1_percent;
    pcfg.thickness_10_percent = inp.thickness_10_percent;

    // Airfoil coordinate file
    pcfg.airfoil_file            = inp.airfoil_file.empty() ? "airfoil.dat"
                                                            : inp.airfoil_file;
    pcfg.auto_calculate_geometry = !inp.airfoil_file.empty();

    // Observer position
    pcfg.observer_distance = inp.observer_distance;
    pcfg.observer_theta    = inp.observer_theta;
    pcfg.observer_phi      = inp.observer_phi;

    // ── Run bladenoise::NoiseCalculator ───────────────────────────────────────
    // BPM path: empty AirfoilData is fine (BL correlations need no geometry).
    // Xfoil path: airfoil x/y coordinates must be present in inp.airfoil_data.
    //   Use AirfoilDataAdapter::Convert() to populate them before calling Build().
    io::AirfoilData airfoil_data;
    if (inp.airfoil_data)
    {
        airfoil_data = *inp.airfoil_data; // pre-converted coordinates supplied
    }
    else
    {
        airfoil_data.num_points = 0;      // BPM: geometry not needed
    }

    noise::NoiseCalculator calculator;
    CombinedNoiseResults bn;
    const bool ok = calculator.calculate(pcfg, airfoil_data, bn);

    if (!ok)
    {
        std::cerr << "  BladeNoiseConfigBuilder: section " << inp.section_index
                  << " failed: " << calculator.get_error() << '\n';
        return false;
    }

    // ── Map CombinedNoiseResults → SectionNoiseResult ─────────────────────────
    // (Spectrum fields only; geometric fields are set by SectionNoiseCalculator)
    auto mapSpectrum = [](SectionNoiseSpectrum &dst, NoiseResult const &src)
    {
        dst.spl.assign(src.spl.begin(), src.spl.end());
        dst.oaspl = src.overall_spl;
    };

    mapSpectrum(result.tbl_pressure_side, bn.tbl_pressure_side);
    mapSpectrum(result.tbl_suction_side,  bn.tbl_suction_side);
    mapSpectrum(result.separation,        bn.separation);
    mapSpectrum(result.laminar_vortex,    bn.laminar_vortex);
    mapSpectrum(result.bluntness,         bn.bluntness);
    mapSpectrum(result.turbulent_inflow,  bn.turbulent_inflow);
    mapSpectrum(result.total,             bn.total);

    return true;
}
