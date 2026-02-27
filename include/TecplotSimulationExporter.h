#pragma once
/**
 * @file TecplotSimulationExporter.h
 * @brief Concrete simulation results exporter writing Tecplot .dat files.
 *
 * SOLID compliance:
 *  S – responsible only for mapping simulation results → DataFormat objects.
 *      Actual formatting and I/O are delegated to IFormatter / IOutputTarget.
 *  O – new output formats: implement ISimulationResultsExporter with a
 *      different IFormatter (e.g. CsvFormatter), zero changes here.
 *  L – satisfies ISimulationResultsExporter fully.
 *  I – consumers see only ISimulationResultsExporter.
 *  D – depends on IFormatter and IOutputTarget abstractions.
 */
#include "ISimulationResultsExporter.h"
#include "IFormatter.h"
#include <memory>

class TecplotSimulationExporter final : public ISimulationResultsExporter
{
public:
    explicit TecplotSimulationExporter(std::shared_ptr<IFormatter> formatter);

    // ── ISimulationResultsExporter ────────────────────────────────────────────
    bool ExportPowerCurve(
        std::vector<PowerCurvePoint> const &power_curve,
        std::string const &output_path) const override;

    bool ExportBladeData(
        BEMPostprocessResult const &pp,
        TurbineGeometry const *turbine,
        double vinf,
        std::string const &output_path) const override;

    bool ExportRotorDiscData(
        std::vector<BEMPostprocessResult> const &pp_vec,
        TurbineGeometry const *turbine,
        std::vector<double> const &vinf_vec,
        std::string const &output_path) const override;

private:
    std::shared_ptr<IFormatter> formatter_;

    /// Build a DataFormat from power curve data.
    static DataFormat BuildPowerCurveFormat(
        std::vector<PowerCurvePoint> const &power_curve);

    /// Build a DataFormat from single-point blade section data.
    static DataFormat BuildBladeDataFormat(
        BEMPostprocessResult const &pp,
        TurbineGeometry const *turbine,
        double vinf);

    /// Build a DataFormat from all wind speeds (rotor disc zones).
    static DataFormat BuildRotorDiscFormat(
        std::vector<BEMPostprocessResult> const &pp_vec,
        TurbineGeometry const *turbine,
        std::vector<double> const &vinf_vec);

    /// Write a fully built DataFormat to a file path.
    bool Write(DataFormat const &fmt, std::string const &path) const;
};
