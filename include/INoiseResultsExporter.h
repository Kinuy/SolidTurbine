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
     * @param results    One BladeNoiseResult per wind speed, ordered by vinf.
     * @param output_path  File path to write.
     */
    virtual bool ExportPowerCurveNoise(
        std::vector<BladeNoiseResult> const &results,
        std::string                   const &output_path) const = 0;
};
