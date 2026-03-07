#pragma once

#include "bladenoise/core/Types.h"
#include "bladenoise/core/ProjectConfig.h"

namespace bladenoise {
namespace noise {

class BluntnessNoiseSource {
public:
    bool calculate(
        const ProjectConfig& config,
        const BoundaryLayerState& upper_bl,
        const BoundaryLayerState& lower_bl,
        const RealVector& frequencies,
        NoiseResult& result);

private:
    Real G4_function(Real hdstar, Real psi) const;
    Real G5_function(Real hdstar, Real eta) const;
};

}  // namespace noise
}  // namespace bladenoise
