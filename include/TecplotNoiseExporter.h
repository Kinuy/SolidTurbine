#pragma once
/**
 * @file TecplotNoiseExporter.h
 * @brief Tecplot .dat exporter for blade noise results.
 *
 * ── ExportBladeNoise ─────────────────────────────────────────────────────────
 *  Single operating point: 7 zones (one per noise source), rows = sections.
 *  Variables: radius, chord, velocity, alpha, OASPL, SPL[34 bands].
 *
 * ── ExportPowerCurveNoise ────────────────────────────────────────────────────
 *  Full power curve: one zone per operating point (named "vinf=XX.Xm/s"),
 *  rows = blade sections.
 *  Variables: v_inf, radius, chord, v_loc, Re, Mach, alpha,
 *             OASPL_total, OASPL_TBL_p, OASPL_TBL_s, OASPL_sep,
 *             OASPL_LBL, OASPL_blunt, OASPL_TI,
 *             LWA_total [dB re 1pW]  (energy-summed over sections × span).
 *
 * SOLID:
 *  S – formats and writes; delegates I/O to IFormatter / DataWriter.
 *  O – new export methods extend INoiseResultsExporter without modifying this.
 *  L – satisfies INoiseResultsExporter fully.
 */
#include "INoiseResultsExporter.h"
#include "IFormatter.h"
#include "DataFormat.h"
#include "RotorNoiseResult.h"
#include <memory>
#include <string>
#include <vector>

class TecplotNoiseExporter final : public INoiseResultsExporter
{
public:
    explicit TecplotNoiseExporter(std::shared_ptr<IFormatter> formatter);

    // ── INoiseResultsExporter ─────────────────────────────────────────────────
    bool ExportBladeNoise(
        BladeNoiseResult const &result,
        std::string      const &output_path) const override;

    bool ExportPowerCurveNoise(
        std::vector<BladeNoiseResult> const &results,
        std::string                   const &output_path) const override;

    /**
     * @brief Export aggregated rotor noise: one zone per noise source,
     *        rows = operating points (v_inf), columns = OASPL/LWA + SPL spectrum.
     */
    bool ExportRotorNoise(
        std::vector<RotorNoiseResult> const &results,
        std::string                   const &output_path) const override;

private:
    std::shared_ptr<IFormatter> formatter_;

    // ── Format builders (SRP: each builds one DataFormat) ────────────────────
    static DataFormat BuildSinglePointFormat(BladeNoiseResult const &result);
    static DataFormat BuildPowerCurveFormat(
        std::vector<BladeNoiseResult> const &results);

    // ── Shared helpers ────────────────────────────────────────────────────────
    /// Variable list for the power-curve noise file.
    static std::vector<std::string> PowerCurveVariables(
        std::vector<double> const &frequencies);

    /// Build one DataZone for one operating point.
    static DataZone BuildOperatingPointZone(BladeNoiseResult const &result);

    /// Sound power level [dB re 1 pW] summed over all sections for one source.
    /// LW = 10·log10( Σ_i  10^(OASPL_i / 10) · chord_i · span_i ) + 120
    static double ComputeLWA(BladeNoiseResult const &result,
                             SectionNoiseSpectrum SectionNoiseResult::*src);

    // ── Rotor noise helpers ───────────────────────────────────────────────────
    /// Variable list for the rotor noise file (OASPL/LWA scalars + SPL spectrum).
    static std::vector<std::string> RotorNoiseVariables(
        std::vector<double> const &frequencies);

    /// Column precisions for rotor noise zones.
    static std::vector<int> RotorNoisePrecisions(int n_bands);

    /// Build one DataZone for one noise source across all operating points.
    static DataZone BuildRotorNoiseSourceZone(
        std::vector<RotorNoiseResult> const &results,
        std::string                   const &zone_title,
        RotorNoiseSourceResult RotorNoiseResult::*src);

    /// Assemble the complete DataFormat for the rotor noise file.
    static DataFormat BuildRotorNoiseFormat(
        std::vector<RotorNoiseResult> const &results);

    bool Write(DataFormat const &fmt, std::string const &path) const;
};
