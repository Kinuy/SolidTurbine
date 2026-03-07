#pragma once
/**
 * @file BEMSectionNoiseAdapter.h
 * @brief Concrete adapter: BEMPostprocessResult + TurbineGeometry
 *        → std::vector<SectionNoiseInput>.
 *
 * This is the only place that knows about both the SolidTurbine domain
 * (BEMPostprocessResult, TurbineGeometry, ISimulationConfig) and the
 * bladenoise input format (SectionNoiseInput).
 *
 * SOLID:
 *  S – maps domain data to noise inputs; no noise physics.
 *  O – new mapping strategies extend ISectionNoiseAdapter.
 *  L – satisfies ISectionNoiseAdapter fully.
 *  D – SectionNoiseCalculator sees only ISectionNoiseAdapter.
 */
#include "ISectionNoiseAdapter.h"
#include "ISimulationConfig.h"

class BEMSectionNoiseAdapter final : public ISectionNoiseAdapter
{
public:
    /**
     * @param observer_distance  Acoustic observer distance [m].
     * @param observer_theta     Observer elevation [deg] from chord line.
     * @param observer_phi       Observer azimuth  [deg] from span line.
     * @param turbulence_intensity     Ambient TI as fraction (not %) [−].
     * @param turbulence_length_scale  Integral length scale [m].
     */
    BEMSectionNoiseAdapter(double observer_distance     = 1.22,
                           double observer_theta        = 90.0,
                           double observer_phi          = 90.0,
                           double turbulence_intensity     = 0.05,
                           double turbulence_length_scale  = 0.01);

    std::vector<SectionNoiseInput> Build(
        BEMPostprocessResult const  &pp,
        TurbineGeometry const       *turbine,
        ISimulationConfig const     &sim_config,
        double                       vinf) const override;

private:
    double observer_distance_;
    double observer_theta_;
    double observer_phi_;
    double turbulence_intensity_;
    double turbulence_length_scale_;
};
