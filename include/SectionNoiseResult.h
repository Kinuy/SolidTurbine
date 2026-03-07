#pragma once
/**
 * @file SectionNoiseResult.h
 * @brief Noise results for one blade section — SolidTurbine-side struct.
 *
 * Mirrors bladenoise::CombinedNoiseResults but uses SolidTurbine types
 * (double, std::vector<double>) so the rest of the project never needs to
 * include any bladenoise header.
 *
 * SOLID:
 *  S – pure data carrier; no logic.
 *  I – exporters depend on this struct only, not on bladenoise internals.
 */
#include <vector>
#include <cstddef>

/// Noise spectrum + OASPL for one source at one blade section.
struct SectionNoiseSpectrum
{
    std::vector<double> spl;        ///< SPL per 1/3-octave band [dB]
    double              oaspl{0.0}; ///< Overall SPL [dB]
};

/// All noise source results for one blade section.
struct SectionNoiseResult
{
    std::size_t section_index{0};
    double      radius{0.0};        ///< Radial position [m]
    double      chord{0.0};         ///< Chord [m]
    double      span{0.0};          ///< Element span (dr) [m]
    double      velocity{0.0};      ///< Local flow velocity [m/s]
    double      mach{0.0};          ///< Local Mach number [-]
    double      reynolds{0.0};      ///< Local Reynolds number [-]
    double      alpha_deg{0.0};     ///< Effective AoA [deg]

    SectionNoiseSpectrum tbl_pressure_side;
    SectionNoiseSpectrum tbl_suction_side;
    SectionNoiseSpectrum separation;
    SectionNoiseSpectrum laminar_vortex;
    SectionNoiseSpectrum bluntness;
    SectionNoiseSpectrum turbulent_inflow;
    SectionNoiseSpectrum total;

    bool converged{false};  ///< false if bladenoise::NoiseCalculator failed
};

/// Full blade noise result: one SectionNoiseResult per section.
struct BladeNoiseResult
{
    std::vector<SectionNoiseResult> sections;
    std::vector<double>             frequencies; ///< 1/3-octave band centres [Hz]
    double                          vinf{0.0};   ///< Wind speed for this operating point [m/s]
};
