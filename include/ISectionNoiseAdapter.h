#pragma once
/**
 * @file ISectionNoiseAdapter.h
 * @brief Interface: maps SolidTurbine domain data → SectionNoiseInput.
 *
 * SOLID:
 *  S – responsible only for the data mapping.
 *  O – alternative adapters (e.g. for wind-tunnel data) implement this
 *      interface without touching SectionNoiseCalculator.
 *  D – SectionNoiseCalculator depends on this abstraction, not on
 *      BEMPostprocessResult / TurbineGeometry / ISimulationConfig directly.
 */
#include "SectionNoiseInput.h"
#include "SectionNoiseResult.h"
#include "BEMPostprocessor.h"   // BEMPostprocessResult
#include "ISimulationConfig.h"
#include <vector>

class TurbineGeometry;
class IBEMSolver;

class ISectionNoiseAdapter
{
public:
    virtual ~ISectionNoiseAdapter() = default;

    /**
     * @brief Build one SectionNoiseInput per blade section.
     *
     * @param pp          Postprocessor result for this operating point.
     * @param turbine     Blade geometry (chord, radius, thickness, …).
     * @param sim_config  Physics constants.
     * @param vinf        Wind speed [m/s] for this operating point.
     * @return            One SectionNoiseInput per blade section.
     */
    virtual std::vector<SectionNoiseInput> Build(
        BEMPostprocessResult const  &pp,
        TurbineGeometry const       *turbine,
        ISimulationConfig const     &sim_config,
        double                       vinf) const = 0;
};
