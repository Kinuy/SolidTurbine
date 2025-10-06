#pragma once

#include <memory>
#include <vector>
#include <Eigen\Core>
#include <Eigen\Geometry>
#include <Eigen\Eigenvalues> 
#include "Surface.h"

/**
 * @brief Class responsible for creation of surface related items like panels  and nodes
 */
class SurfaceCreator{

private:

public:

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	/**
	 * @brief Constructor
	 */
	SurfaceCreator(Surface& surface);

    /**
    * @brief
    */
    Surface& surface;

    /**
    * @brief
    */
    std::vector<int> createNodesForPoints(const std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> >& points);

    /**
    * @brief
    */
    std::vector<int> createPanelsBetweenShapes(const std::vector<int>& firstNodes, const std::vector<int>& secondNodes, bool cyclic = true);

    /**
    * @brief
    */
    std::vector<int> createPanelsInsideShape(const std::vector<int>& nodes, const Eigen::Vector3d& tipPoint, int zSign);

    /**
    * @brief
    */
    void finish();

};

