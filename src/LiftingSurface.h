#pragma once

#include <Eigen\Core>

#include "Surface.h"

/**
 * @brief Parameters defining the characteristics of a lifting surface
 */
class LiftingSurface : public Surface {

private:

    /**
    * @brief
    */
    Eigen::MatrixXd trailingEdgeBisectors;

    /**
    * @brief
    */
    Eigen::MatrixXd wakeNormals;

public:

	/**
	 * @brief Constructor
	 * @param id Identifier for the lifting surface
	 */
	LiftingSurface(const std::string& id);

	/**
	 * @brief Destructor
	 */
	~LiftingSurface();

    /**
    * @brief
    */
    Eigen::MatrixXi nodesTop;

    /**
    * @brief
    */
    Eigen::MatrixXi nodesBottom;

    /**
    * @brief
    */
    Eigen::MatrixXi panelsTop;

    /**
    * @brief
    */
    Eigen::MatrixXi panelsBottom;

    /**
    * @brief
    */
    int getChordwiseNodesCount() const;

    /**
    * @brief
    */
    int getChordwisePanelsCount() const;

    /**
    * @brief
    */
    int getSpanwiseNodesCount() const;

    /**
    * @brief
    */
    int getSpanwisePanelsCount() const;

    /**
    * @brief
    */
    int trailingEdgeNode(int index) const;

    /**
    * @brief
    */
    int trailingEdgeTopPanel(int index) const;

    /**
    * @brief
    */
    int trailingEdgeBottomPanel(int index) const;

    /**
    * @brief
    */
    void finishTrailingEdge();

    /**
    * @brief
    */
    virtual void transform(const Eigen::Transform<double, 3, Eigen::Affine>& transformation);

    /**
    * @brief
    */
    virtual Eigen::Vector3d wakeEmissionVelocity(const Eigen::Vector3d& apparent_velocity, int node_index) const;

};
