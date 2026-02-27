#pragma once

#include <memory>
#include "BoundaryLayer.h"
#include "BladeBodyManager.h"

/**
 * @brief Store for blade body and boundary layer related data
 */
struct BladeBodyData {

    /**
    * @brief
    */
    BladeBodyData(std::shared_ptr<BladeBodyManager> bladeBody, std::shared_ptr<BoundaryLayer> boundaryLayer) :
        bladeBody(bladeBody), boundaryLayer(boundaryLayer) {
    }

    /**
    * @brief
    */
    std::shared_ptr<BladeBodyManager> bladeBody;

    /**
    * @brief
    */
    std::shared_ptr<BoundaryLayer> boundaryLayer;
};
