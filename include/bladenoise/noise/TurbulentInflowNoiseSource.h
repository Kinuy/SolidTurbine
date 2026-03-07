#pragma once

#include "bladenoise/core/Types.h"
#include "bladenoise/core/ProjectConfig.h"

namespace bladenoise {
namespace noise {

class TurbulentInflowNoiseSource {
public:
    explicit TurbulentInflowNoiseSource(TINoiseMethod method = TINoiseMethod::AMIET);

    bool calculate(
        const ProjectConfig& config,
        const BoundaryLayerState& upper_bl,
        const BoundaryLayerState& lower_bl,
        const RealVector& frequencies,
        NoiseResult& result);

private:
    Real von_karman_spectrum(Real k1, Real length_scale,
                            Real turbulence_intensity, Real velocity) const;
    Real amiet_response(Real k_chord, Real mach) const;
    Real thickness_correction(Real freq, Real chord, Real velocity,
                             Real t_1p, Real t_10p) const;

    TINoiseMethod method_;
};

}  // namespace noise
}  // namespace bladenoise
