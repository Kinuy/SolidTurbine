#pragma once
/**
 * @file ISimulationResultsExporter.h
 * @brief Abstract interface for simulation results export.
 *
 * OCP: new export formats are added by implementing this interface,
 * not by modifying existing exporters.
 * DIP: main.cpp depends on this abstraction, not on TecplotExporter directly.
 */
#include <vector>
#include "OperationSolver.h"      // PowerCurvePoint
#include "BEMPostprocessor.h"     // BEMPostprocessResult

class TurbineGeometry;

class ISimulationResultsExporter
{
public:
    virtual ~ISimulationResultsExporter() = default;

    /// Export the full power curve (one row per wind speed).
    virtual bool ExportPowerCurve(
        std::vector<PowerCurvePoint> const &power_curve,
        std::string const &output_path) const = 0;

    /// Export per-section blade data for one operating point.
    virtual bool ExportBladeData(
        BEMPostprocessResult const &pp,
        TurbineGeometry const *turbine,
        double vinf,
        std::string const &output_path) const = 0;

    /// Export per-section blade data for all wind speeds (rotor disc view).
    virtual bool ExportRotorDiscData(
        std::vector<BEMPostprocessResult> const &pp_vec,
        TurbineGeometry const *turbine,
        std::vector<double> const &vinf_vec,
        std::string const &output_path) const = 0;
};
