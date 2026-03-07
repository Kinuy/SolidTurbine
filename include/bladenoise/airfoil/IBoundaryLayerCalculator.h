#pragma once

#include "bladenoise/core/Types.h"
#include "bladenoise/core/ProjectConfig.h"
#include "bladenoise/io/IOTypes.h"
#include <memory>
#include <string>

namespace bladenoise {
namespace airfoil {

// Abstract interface for boundary layer calculators
class IBoundaryLayerCalculator {
public:
    virtual ~IBoundaryLayerCalculator() = default;

    virtual bool calculate(
        const io::AirfoilData& airfoil,
        const ProjectConfig& config,
        BoundaryLayerState& upper_bl,
        BoundaryLayerState& lower_bl) = 0;

    virtual std::string get_error() const = 0;
};

// Factory function
std::unique_ptr<IBoundaryLayerCalculator>
create_boundary_layer_calculator(BoundaryLayerMethod method);

}  // namespace airfoil
}  // namespace bladenoise
