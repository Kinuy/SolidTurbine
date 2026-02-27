#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <Eigen/Core>

#include "SurfaceData.h"
#include "LiftingSurfaceData.h"
#include "Surface.h"
#include "LiftingSurface.h"
#include "Wake.h"
#include "SurfacePanelEdgeData.h"
#include "CompareSurfacePanelEdge.h"

/**
 * @brief Manages components of blade like surfaces , panels and nodes
 */
class BladeBodyManager
{

private:
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW // Ensures proper memory alignment

        /**
         * @brief
         */
        std::string id;

    /**
     * @brief
     */
    std::vector<std::shared_ptr<SurfaceData>> nonLiftingSurfaces;

    /**
     * @brief
     */
    std::vector<std::shared_ptr<LiftingSurfaceData>> liftingSurfaces;

    /**
     * @brief
     */
    BladeBodyManager(const std::string &id);

    /**
     * @brief
     */
    ~BladeBodyManager();

    /**
     * @brief
     */
    void addNonLiftingSurface(std::shared_ptr<Surface> surface);

    /**
     * @brief
     */
    void addLiftingSurface(std::shared_ptr<LiftingSurface> liftingSurface);

    /**
     * @brief
     */
    void addLiftingSurface(std::shared_ptr<LiftingSurface> liftingSurface, std::shared_ptr<Wake> wake);

    /**
     * @brief
     */
    void stitchPanels(std::shared_ptr<Surface> surfaceA, int panelA, int edgeA, std::shared_ptr<Surface> surfaceB, int panelB, int edgeB);

    /**
     * @brief
     */
    std::vector<SurfacePanelEdgeData> panelNeighbors(const std::shared_ptr<Surface> &surface, int panel) const;

    /**
     * @brief
     */
    std::vector<SurfacePanelEdgeData> panelNeighbors(const std::shared_ptr<Surface> &surface, int panel, int edge) const;

    /**
     * @brief
     */
    Eigen::Vector3d position;

    /**
     * @brief
     */
    Eigen::Vector3d velocity;

    /**
     * @brief
     */
    Eigen::Quaterniond attitude;

    /**
     * @brief
     */
    Eigen::Vector3d rotationalVelocity;

    /**
     * @brief
     */
    void setPosition(const Eigen::Vector3d &position);

    /**
     * @brief
     */
    void setAttitude(const Eigen::Quaterniond &attitude);

    /**
     * @brief
     */
    void setVelocity(const Eigen::Vector3d &velocity);

    /**
     * @brief
     */
    void setRotationalVelocity(const Eigen::Vector3d &rotationalVelocity);

    /**
     * @brief
     */
    Eigen::Vector3d panelKinematicVelocity(const std::shared_ptr<Surface> &surface, int panel) const;

    /**
     * @brief
     */
    Eigen::Vector3d nodeKinematicVelocity(const std::shared_ptr<Surface> &surface, int node) const;

protected:
    /**
     * @brief
     */
    std::map<SurfacePanelEdgeData, std::vector<SurfacePanelEdgeData>, CompareSurfacePanelEdge> stitches;
};
