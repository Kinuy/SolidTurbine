#pragma once

#include <vector>
#include <memory>

#include <Eigen\Core>
#include <Eigen\Geometry>

#include "LiftingSurface.h"

/**
 * @brief Manages wake characteristics for aerodynamic surfaces
 */
class Wake : public Surface {

private:

public:

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /**
    * @brief
    */
    Wake(std::shared_ptr<LiftingSurface> liftingSurface);

    /**
    * @brief
    */
    std::shared_ptr<LiftingSurface> liftingSurface;

    /**
    * @brief
    */
    std::vector<double> doubletCoefficients;

    /**
    * @brief
    */
    void translateTrailingEdge(const Eigen::Vector3d& translation);

    /**
    * @brief
    */
    void transformTrailingEdge(const Eigen::Transform<double, 3, Eigen::Affine>& transformation);

    /**
    * @brief
    */
    virtual void updateProperties(double dt);

    /**
    * @brief
    */
    virtual void addLayer();

    /**
    * @brief
    */
    virtual Eigen::Vector3d vortexRingUnitVelocity(const Eigen::Vector3d& x, int panel) const;

};





