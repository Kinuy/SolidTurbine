#pragma once

#include "bladenoise/core/Types.h"
#include "bladenoise/core/ProjectConfig.h"

namespace bladenoise {
namespace noise {

/**
 * @brief Interface for noise source calculation models
 * 
 * Follows Dependency Inversion Principle - high-level noise calculation
 * depends on this abstraction, not concrete implementations.
 */
class INoiseSource {
public:
    virtual ~INoiseSource() = default;

    /**
     * @brief Calculate noise spectrum for this source
     * @param config Project configuration
     * @param upper_bl Upper surface boundary layer state
     * @param lower_bl Lower surface boundary layer state
     * @param frequencies Frequency band centers
     * @param result Output noise result
     * @return true if calculation succeeded
     */
    [[nodiscard]] virtual bool calculate(
        const ProjectConfig& config,
        const BoundaryLayerState& upper_bl,
        const BoundaryLayerState& lower_bl,
        const RealVector& frequencies,
        NoiseResult& result
    ) = 0;

    /**
     * @brief Get name of this noise source
     */
    [[nodiscard]] virtual std::string name() const = 0;
};

}  // namespace noise
}  // namespace bladenoise
