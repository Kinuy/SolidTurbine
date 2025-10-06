#pragma once

#include <Eigen\Core>
#include <memory>
#include "Surface.h"

/**
 * @brief Store for point data on surface panels
 */
struct SurfacePanelPointData {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /**
    * @brief
    */
    SurfacePanelPointData(std::shared_ptr<Surface> surface, int panel, const Eigen::Vector3d& point)
        : surface(surface), panel(panel), point(point) {
    };

    /**
    * @brief
    */
    std::shared_ptr<Surface> surface;

    /**
    * @brief
    */
    int panel;

    /**
    * @brief
    */
    Eigen::Vector3d point;
};
