#include "bladenoise/airfoil/IBoundaryLayerCalculator.h"
#include "bladenoise/airfoil/BPMBoundaryLayerCalculator.h"
#include "bladenoise/airfoil/XfoilBoundaryLayerCalculator.h"

namespace bladenoise {
namespace airfoil {

std::unique_ptr<IBoundaryLayerCalculator> 
create_boundary_layer_calculator(BoundaryLayerMethod method) {
    switch (method) {
        case BoundaryLayerMethod::BPM:
            return std::make_unique<BPMBoundaryLayerCalculator>();
        case BoundaryLayerMethod::XFOIL:
            return std::make_unique<XfoilBoundaryLayerCalculator>();
        default:
            return std::make_unique<BPMBoundaryLayerCalculator>();
    }
}

}  // namespace airfoil
}  // namespace bladenoise
