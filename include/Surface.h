#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <numbers>
#include <memory>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "LiftingSurfaceParameters.h"
#include "MathUtility.h"

/**
 * @brief Class managing aerodynamic surfaces
 */
class Surface
{

private:
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /**
     * @brief Constructor
     * @param id Identifier for the surface
     */
    Surface(const std::string &id);

    /**
     * @brief Destructor
     */
    virtual ~Surface();

    /**
     * @brief Identifier for the surface
     */
    std::string id;

    /**
     * @brief
     */
    std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d>> nodes;

    /**
     * @brief
     */
    std::vector<std::shared_ptr<std::vector<int>>> panelNodeNeighbors;

    /**
     * @brief
     */
    std::vector<std::vector<int>> panelNodes;

    /**
     * @brief
     */
    std::vector<std::vector<std::vector<std::pair<int, int>>>> panelNeighbors;

    /**
     * @brief
     */
    std::vector<std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d>>> transformedPanelPoints;

    /**
     * @brief
     */
    void rotate(const Eigen::Vector3d &axis, double angle);

    /**
     * @brief
     */
    virtual void transform(const Eigen::Matrix3d &transformation);

    /**
     * @brief
     */
    virtual void transform(const Eigen::Transform<double, 3, Eigen::Affine> &transformation);

    /**
     * @brief
     */
    virtual void translate(const Eigen::Vector3d &translation);

    /**
     * @brief
     */
    const Eigen::Vector3d &panelCollocationPoint(int panel, bool bottomSurface) const;

    /**
     * @brief
     */
    const Eigen::Vector3d &panelNormal(int panel) const;

    /**
     * @brief
     */
    const Eigen::Transform<double, 3, Eigen::Affine> &panelCoordinateTransformation(int panel) const;

    /**
     * @brief
     */
    double panelSurfaceArea(int panel) const;

    /**
     * @brief
     */
    virtual void sourceAndDoubletInfluence(const Eigen::Vector3d &x, int this_panel, double &sourceInfluence, double &doubletInfluence) const;

    /**
     * @brief
     */
    void sourceAndDoubletInfluence(const std::shared_ptr<Surface> &other, int other_panel, int this_panel, double &sourceInfluence, double &doubletInfluence) const;

    /**
     * @brief
     */
    double sourceInfluence(const Eigen::Vector3d &x, int this_panel) const;

    /**
     * @brief
     */
    double sourceInfluence(const std::shared_ptr<Surface> &other, int other_panel, int this_panel) const;

    /**
     * @brief
     */
    double doubletInfluence(const Eigen::Vector3d &x, int this_panel) const;

    /**
     * @brief
     */
    double doubletInfluence(const std::shared_ptr<Surface> &other, int other_panel, int this_panel) const;

    /**
     * @brief
     */
    virtual Eigen::Vector3d sourceUnitVelocity(const Eigen::Vector3d &x, int this_panel) const;

    /**
     * @brief
     */
    Eigen::Vector3d sourceUnitVelocity(const std::shared_ptr<Surface> &other, int other_panel, int this_panel) const;

    /**
     * @brief
     */
    virtual Eigen::Vector3d vortexRingUnitVelocity(const Eigen::Vector3d &x, int this_panel) const;

    /**
     * @brief
     */
    Eigen::Vector3d vortexRingUnitVelocity(const std::shared_ptr<Surface> &other, int other_panel, int this_panel) const;

    /**
     * @brief
     */
    int addTriangle(int nodeA, int nodeB, int nodeC);

    /**
     * @brief
     */
    int addQuadrangle(int nodeA, int nodeB, int nodeC, int nodeD);

    /**
     * @brief
     */
    void calcTopology();

    /**
     * @brief
     */
    void calcGeometry(int panel);

    /**
     * @brief
     */
    void calcGeometry();

    /**
     * @brief
     */
    void cutPanels(int panelA, int panelB);

    /**
     * @brief
     */
    int getNumberOfNodes() const;

    /**
     * @brief
     */
    int getNumberOfPanels() const;

protected:
    /**
     * @brief
     */
    std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d>> panelCollocationPoints[2];

    /**
     * @brief
     */
    std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d>> panelNormals;

    /**
     * @brief
     */
    std::vector<Eigen::Transform<double, 3, Eigen::Affine>, Eigen::aligned_allocator<Eigen::Transform<double, 3, Eigen::Affine>>> panelCoordinateTransformations;

    /**
     * @brief
     */
    std::vector<double> panelSurfaceAreas;
};
