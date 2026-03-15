#pragma once
/**
 * @file RotorNoiseResult.h
 * @brief Aggregated rotor-level noise result for one operating point.
 *
 * Produced by RotorNoiseAggregator from a BladeNoiseResult.
 * All levels are for the complete rotor (n_blades already included).
 *
 * Naming convention follows the Python BladeNoise post-processing:
 *   SPL  – Sound Pressure Level  [dB]   at observer
 *   LW   – Sound Power Level     [dB re 1 pW]
 *   A    – A-weighted            [dB(A)]
 *
 * SOLID:
 *   S – pure data carrier; no I/O or computation.
 */
#include <vector>

/// Aggregated rotor noise for one source (or total) at one operating point.
struct RotorNoiseSourceResult
{
    /// SPL spectrum at observer, energy-summed over all blade sections [dB].
    std::vector<double> spl_spectrum;

    /// A-weighted SPL spectrum [dB(A)].
    std::vector<double> splA_spectrum;

    /// Overall SPL (energy-sum over all freq bands) [dB].
    double oaspl{-100.0};

    /// Overall A-weighted SPL [dB(A)].
    double oasplA{-100.0};

    /// Overall Sound Power Level (energy-sum over all freq bands) [dB re 1 pW].
    double lw{-100.0};

    /// Overall A-weighted Sound Power Level [dB(A) re 1 pW].
    double lwA{-100.0};
};

/// Full rotor noise result for one operating point of the power curve.
struct RotorNoiseResult
{
    double vinf{0.0};           ///< Wind speed [m/s]
    double observer_distance{0.0}; ///< Slant distance to observer [m]
    int    n_blades{3};         ///< Number of rotor blades

    std::vector<double> frequencies; ///< 1/3-octave band centres [Hz]

    // ── Per-source aggregated results ─────────────────────────────────────────
    RotorNoiseSourceResult tbl_pressure_side;
    RotorNoiseSourceResult tbl_suction_side;
    RotorNoiseSourceResult separation;
    RotorNoiseSourceResult laminar_vortex;
    RotorNoiseSourceResult bluntness;
    RotorNoiseSourceResult turbulent_inflow;
    RotorNoiseSourceResult total;   ///< Energy sum of all active sources
};
