#pragma once
#include "bladenoise/io/IOTypes.h"
#include <memory>
#include <optional>
/**
 * @file SectionNoiseInput.h
 * @brief Plain data struct carrying everything needed to compute noise for
 *        one blade section.  Decouples SolidTurbine domain types from the
 *        bladenoise library so neither side depends on the other.
 *
 * SOLID:
 *  S – pure data carrier; no logic.
 *  I – SectionNoiseCalculator depends on this struct, not on BEMPostprocessResult
 *      or TurbineGeometry directly (those are mapped upstream in the adapter).
 */
#include <string>

/// All per-section inputs required by bladenoise::noise::NoiseCalculator.
struct SectionNoiseInput
{
    // ── Atmospheric constants (copied from ISimulationConfig) ─────────────────
    double speed_of_sound      = 340.46;    ///< [m/s]
    double kinematic_viscosity = 1.4607e-5; ///< [m²/s]
    double air_density         = 1.225;     ///< [kg/m³]

    // ── Blade / flow geometry ─────────────────────────────────────────────────
    double chord          = 1.0;   ///< Chord length [m]
    double span           = 1.0;   ///< Element span (dr) [m]
    double velocity       = 0.0;   ///< Local flow velocity [m/s]
    double alpha_deg      = 0.0;   ///< Effective AoA [deg]
    double mach           = 0.0;   ///< Local Mach number [-]
    double reynolds       = 0.0;   ///< Local Reynolds number [-]

    // ── Trailing edge geometry ────────────────────────────────────────────────
    double te_thickness   = 0.0;   ///< TE thickness [m]  (0 → auto from airfoil)
    double te_angle_deg   = 14.0;  ///< TE solid angle PSI [deg]

    // ── Turbulence (for TI noise) ─────────────────────────────────────────────
    double turbulence_intensity    = 0.05; ///< [-] (fraction, not percent)
    double turbulence_length_scale = 0.01; ///< [m]

    // ── Thickness at specific chord locations (for TI noise) ─────────────────
    double thickness_1_percent  = 0.0;  ///< t/c at 1% chord  (0 → use relative thickness)
    double thickness_10_percent = 0.0;  ///< t/c at 10% chord (0 → use relative thickness)

    // ── Airfoil coordinate file (Selig format) ────────────────────────────────
    std::string airfoil_file;      ///< Path to .dat file; empty → skip Xfoil BL method

    /// Pre-converted airfoil coordinates for XfoilBoundaryLayerCalculator.
    /// Null when BPM method is used.  Populated by BEMSectionNoiseAdapter
    /// via AirfoilDataAdapter::Convert() when bl_method == XFOIL.
    std::shared_ptr<bladenoise::io::AirfoilData> airfoil_data;

    // ── Observer position ─────────────────────────────────────────────────────
    double observer_distance = 1.22; ///< [m]
    double observer_theta    = 90.0; ///< [deg] from chord line
    double observer_phi      = 90.0; ///< [deg] from span line

    // ── Section index (for diagnostics) ──────────────────────────────────────
    std::size_t section_index = 0;
};
