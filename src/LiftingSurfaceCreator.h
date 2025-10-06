#pragma once

#include <memory>

#include <Eigen\Core>

#include "LiftingSurface.h"
#include "SurfaceCreator.h"

/**
 * @brief Class responsible for creation of surface related items like panels  and nodes for performance calculation purposes
 */
class LiftingSurfaceCreator : public SurfaceCreator {

private:

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /**
    * @brief
    */
    LiftingSurfaceCreator(LiftingSurface& liftingSurface);

    /**
    * @brief
    */
    LiftingSurface& liftingSurface;

    /**
    * @brief
    */
    std::vector<int> createPanelsInsideAirfoil(const std::vector<int>& airfoilNodes, int trailingEdgePointId, int zSign);

    /**
    * @brief
    */
    void finish(const std::vector<std::vector<int> >& nodeStrips, const std::vector<std::vector<int> >& panelStrips, int trailingEdgePointId);
};
