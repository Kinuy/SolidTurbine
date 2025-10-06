#pragma once

#include <vector>
#include <memory>

#include <Eigen\Core>

#include "Wake.h"
#include "LiftingSurface.h"
#include "MathUtility.h"

/**
 * @brief Ramasamy Leishman wake model implementation
 */
class RLWakeModel : Wake{

private:

    /**
    * @brief
    */
    std::vector<std::vector<double> > baseEdgeLengths;

    /**
    * @brief
    */
    std::vector<std::vector<double> > vortexCoreRadii;

    /**
    * @brief
    */
    void updateVortexRingRadii(int panel, double dt);

public:

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /**
    * @brief
    */
    RLWakeModel(std::shared_ptr<LiftingSurface> liftingSurface);

    /**
    * @brief
    */
    void addLayer();

    /**
    * @brief
    */
    void updateProperties(double dt);

    /**
    * @brief
    */
    Eigen::Vector3d vortexRingUnitVelocity(const Eigen::Vector3d& x, int panel) const;

};
