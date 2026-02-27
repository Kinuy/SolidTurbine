#pragma once

#include <memory>
#include <Eigen/Core>

#include "Surface.h"

/**
 * @brief Manages boundary layer characteristics for aerodynamic surfaces
 */
class BoundaryLayer
{

private:
public:
    /**
     * @brief
     */
    bool recalculate(const Eigen::Vector3d &freestreamVelocity, const Eigen::MatrixXd &surfaceVelocities);

    /**
     * @brief
     */
    double thickness(const std::shared_ptr<Surface> &surface, int panel) const;

    /**
     * @brief
     */
    Eigen::Vector3d velocity(const std::shared_ptr<Surface> &surface, int panel, double y) const;

    /**
     * @brief
     */
    double transpirationVelocity(const std::shared_ptr<Surface> &surface, int panel) const;

    /**
     * @brief
     */
    Eigen::Vector3d friction(const std::shared_ptr<Surface> &surface, int panel) const;
};