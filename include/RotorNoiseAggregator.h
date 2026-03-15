#pragma once
/**
 * @file RotorNoiseAggregator.h
 * @brief Aggregates per-section blade noise into rotor-level SPL and LWA.
 *
 * Implements the acoustic post-processing pipeline from the Python BladeNoise
 * tool (post_processing_NAFNoise_results_New), translated to C++:
 *
 *   1. A-weighting per 1/3-octave band (IEC 61672 / ISO 226 formula).
 *   2. Geometric spreading attenuation  As = 10·log10(1/(4π·d²))  [free field,
 *      Q=1, no room constant — matches the Python tool's default path].
 *   3. Per-source, per-section  SPL → LW:  lw = spl − As.
 *   4. Energy-sum over blade sections per frequency band → blade SPL spectrum.
 *   5. Energy-sum over frequency bands → OASPL / OASPL_A per source.
 *   6. Multi-blade scaling:  LWA_rotor = LWA_blade + 10·log10(n_blades).
 *      (SPL at observer is obtained from the single-blade section SPLs which
 *       already include the observer distance through NAFNoise; the n_blades
 *       contribution is therefore applied at the power-level stage only.)
 *
 * Observer geometry (matches Python BladeNoise):
 *   d = sqrt((hub_height - mic_height)² + (ground_distance + overhang)²)
 *   mic_height = 0 m  (ground-level microphone per IEC 61400-11)
 *
 * SOLID:
 *   S – only computes aggregated noise; no I/O.
 *   O – extend by sub-classing or replacing for different propagation models.
 *   D – depends only on BladeNoiseResult / RotorNoiseResult value types.
 */
#include "SectionNoiseResult.h"
#include "RotorNoiseResult.h"
#include <vector>

class RotorNoiseAggregator
{
public:
    /**
     * @brief Construct aggregator with rotor and observer geometry.
     *
     * @param n_blades          Number of rotor blades.
     * @param hub_height        Hub centre height above ground [m].
     * @param ground_distance   Horizontal distance from tower base to observer [m].
     * @param overhang          Rotor overhang from tower axis [m].
     */
    RotorNoiseAggregator(int    n_blades,
                         double hub_height,
                         double ground_distance,
                         double overhang);

    /**
     * @brief Aggregate one operating point's blade noise into rotor-level result.
     *
     * @param blade_result  Per-section noise from SectionNoiseCalculator.
     * @return RotorNoiseResult with all sources aggregated for this vinf.
     */
    RotorNoiseResult Aggregate(BladeNoiseResult const &blade_result) const;

    /// Observer slant distance computed from constructor geometry [m].
    double ObserverDistance() const { return observer_distance_; }

private:
    int    n_blades_;
    double observer_distance_;  ///< Pre-computed slant distance [m]

    // ── Internal helpers ──────────────────────────────────────────────────────

    /**
     * @brief Compute A-weighting correction [dB] for each frequency.
     *  A(f) = 2 + 20·log10(RA(f))  with the standard polynomial approximation.
     */
    static std::vector<double> ComputeAWeights(
        std::vector<double> const &frequencies);

    /**
     * @brief Geometric spreading attenuation [dB] for free-field propagation.
     *  As = 10·log10(1 / (4·π·d²))
     *  (negative value — subtract to convert SPL → LW)
     */
    static double GeometricAttenuation(double observer_distance);

    /**
     * @brief Energy-sum a list of dB values: 10·log10(Σ 10^(x_i/10)).
     */
    static double EnergySum(std::vector<double> const &levels);

    /**
     * @brief Aggregate one noise source (pointer-to-member) across all sections.
     *
     * For each frequency band: energy-sum the per-section SPL values.
     * Then derive OASPL, OASPL_A, LW, LWA.
     *
     * @param sections   All blade sections for this operating point.
     * @param src        Pointer-to-member selecting the noise source.
     * @param a_weights  Per-frequency A-weighting corrections [dB].
     * @param As         Geometric attenuation [dB] (negative number).
     * @param n_blades   Rotor blade count for LWA scaling.
     * @return Fully populated RotorNoiseSourceResult.
     */
    static RotorNoiseSourceResult AggregateSource(
        std::vector<SectionNoiseResult> const &sections,
        SectionNoiseSpectrum SectionNoiseResult::*src,
        std::vector<double>  const &a_weights,
        double               As,
        int                  n_blades);
};
