#pragma once
/**
 * @file INoiseResultsExporter.h
 * @brief Abstract interface for exporting blade noise results.
 *
 * SOLID:
 *  O – new formats (CSV, VTK …) implement this without touching existing code.
 *  D – main.cpp depends on this abstraction, not on concrete exporters.
 *
 * ── Methods ──────────────────────────────────────────────────────────────────
 *  ExportBladeNoise        – single operating point (one BladeNoiseResult).
 *  ExportPowerCurveNoise   – full power curve (one BladeNoiseResult per vinf),
 *                            written as one Tecplot zone per operating point,
 *                            rows = blade sections.
 */
#include "SectionNoiseResult.h"
#include "RotorNoiseResult.h"
#include <string>
#include <vector>

class INoiseResultsExporter
{
public:
    virtual ~INoiseResultsExporter() = default;

    /// Export noise for a single operating point.
    virtual bool ExportBladeNoise(
        BladeNoiseResult const &result,
        std::string      const &output_path) const = 0;

    /**
     * @brief Export noise for all operating points on the power curve.
     *
     * Output layout: one zone per operating point (named by v_inf),
     * rows = blade sections, columns = per-section noise quantities.
     *
     * @param results      One BladeNoiseResult per wind speed, ordered by vinf.
     * @param output_path  File path to write.
     */
    virtual bool ExportPowerCurveNoise(
        std::vector<BladeNoiseResult> const &results,
        std::string                   const &output_path) const = 0;

    /**
     * @brief Export aggregated rotor noise for all power curve operating points.
     *
     * Output layout: one zone per noise source; rows = operating points.
     * Columns: v_inf, observer_distance, OASPL [dB], OASPL_A [dB(A)],
     *          LW [dB re 1pW], LWA [dB(A) re 1pW],
     *          SPL spectrum [dB], SPL_A spectrum [dB(A)].
     *
     * @param results      One RotorNoiseResult per wind speed (from RotorNoiseAggregator).
     * @param output_path  File path to write.
     */
    virtual bool ExportRotorNoise(
        std::vector<RotorNoiseResult> const &results,
        std::string                   const &output_path) const = 0;
};
