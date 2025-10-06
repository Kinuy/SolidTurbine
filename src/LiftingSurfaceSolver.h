#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <sstream>
#include <iostream>
#include <filesystem>

#include <Eigen\Core>
#include <Eigen\Geometry>
#include <Eigen\Dense>
#include <Eigen\Sparse>
#include <Eigen\IterativeLinearSolvers>
#include <Eigen\LU>

#include "BladeBodyManager.h"
#include "BladeBodyData.h"
#include "SurfaceData.h"
#include "SurfacePanelPointData.h"
#include "BoundaryLayer.h"
#include "Surface.h"
#include "LiftingSurface.h"
#include "LiftingSurfaceData.h"
#include "Wake.h"
#include "SurfaceWriter.h"
#include "SurfacePanelEdgeData.h"
#include "CompareSurfacePanelEdge.h"
#include "LiftingSurfaceParameters.h"


/**
 * @brief Solver class for aerodynamic lifting surface calculations
 */
class LiftingSurfaceSolver{

private:

    /**
    * @brief
    */
    std::string logFolder;

    /**
    * @brief
    */
    std::vector<std::shared_ptr<SurfaceData> > nonWakeSurfaces;

    /**
    * @brief
    */
    int nonWakePanels;

    /**
    * @brief
    */
    std::map<std::shared_ptr<Surface>, std::shared_ptr<BladeBodyData> > surfaceToBody;

    /**
    * @brief
    */
    Eigen::VectorXd sourceCoefficients;

    /**
    * @brief
    */
    Eigen::VectorXd doubletCoefficients;

    /**
    * @brief
    */
    Eigen::VectorXd surfaceVelocityPotentials;

    /**
    * @brief
    */
    Eigen::MatrixXd surfaceVelocities;

    /**
    * @brief
    */
    Eigen::VectorXd pressureCoefficients;

    /**
    * @brief
    */
    Eigen::VectorXd previousSurfaceVelocityPotentials;

    /**
    * @brief
    */
    double computeSourceCoefficient(const std::shared_ptr<BladeBodyManager>& body, const std::shared_ptr<Surface>& surface, int panel,
        const std::shared_ptr<BoundaryLayer>& boundaryLayer, bool includeWakeInfluence) const;

    /**
    * @brief
    */
    double computeSurfaceVelocityPotential(const std::shared_ptr<Surface>& surface, int offset, int panel) const;

    /**
    * @brief
    */
    double computeSurfaceVelocityPotentialTimeDerivative(int offset, int panel, double dt) const;

    /**
    * @brief
    */
    Eigen::Vector3d computeSurfaceVelocity(const std::shared_ptr<BladeBodyManager>& bladeBody, const std::shared_ptr<Surface>& surface, int panel) const;

    /**
    * @brief
    */
    double computeReferenceVelocitySquared(const std::shared_ptr<BladeBodyManager>& bladeBody) const;

    /**
    * @brief
    */
    double computePressureCoefficient(const Eigen::Vector3d& surfaceVelocity, double dphidt, double v_ref) const;

    /**
    * @brief
    */
    Eigen::Vector3d computeVelocityInterpolated(const Eigen::Vector3d& x, std::set<int>& ignoreSet) const;

    /**
    * @brief
    */
    Eigen::Vector3d computeVelocity(const Eigen::Vector3d& x) const;

    /**
    * @brief
    */
    double computeVelocityPotential(const Eigen::Vector3d& x) const;

    /**
    * @brief
    */
    Eigen::Vector3d computeTrailingEdgeVortexDisplacement(const std::shared_ptr<BladeBodyManager>& body, const std::shared_ptr<LiftingSurface>& liftingSurface, int index, double dt) const;

    /**
    * @brief
    */
    Eigen::Vector3d computeScalarFieldGradient(const Eigen::VectorXd& scalarField, const std::shared_ptr<BladeBodyManager>& body, const std::shared_ptr<Surface>& surface, int panel) const;

    /**
    * @brief
    */
    int computeIndex(const std::shared_ptr<Surface>& surface, int panel) const;

public:

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /**
    * @brief
    */
    LiftingSurfaceSolver(const std::string& logFolder);

    /**
    * @brief
    */
    ~LiftingSurfaceSolver();

    /**
    * @brief
    */
    void dirHandler(const std::string& folder);

    /**
    * @brief
    */
    std::vector<std::shared_ptr<BladeBodyData> > bodies;

    /**
    * @brief
    */
    void addBody(std::shared_ptr<BladeBodyManager> bladeBody);

    /**
    * @brief
    */
    void addBody(std::shared_ptr<BladeBodyManager> bladeBody, std::shared_ptr<BoundaryLayer> boundaryLayer);

    /**
    * @brief
    */
    Eigen::Vector3d freestream_velocity;

    /**
    * @brief
    */
    void setFreestreamVelocity(const Eigen::Vector3d& value);

    /**
    * @brief
    */
    double fluidDensity;

    /**
    * @brief
    */
    void setFluidDensity(double value);

    /**
    * @brief
    */
    void initializeWakes(double dt = 0.0);

    /**
    * @brief
    */
    void updateWakes(double dt = 0.0);

    /**
    * @brief
    */
    bool solve(double dt = 0.0, bool propagate = true);

    /**
    * @brief
    */
    void propagate();

    /**
    * @brief
    */
    double velocityPotential(const Eigen::Vector3d& x) const;

    /**
    * @brief
    */
    Eigen::Vector3d velocity(const Eigen::Vector3d& x) const;

    /**
    * @brief
    */
    double surfaceVelocityPotential(const std::shared_ptr<Surface>& surface, int panel) const;

    /**
    * @brief
    */
    Eigen::Vector3d surfaceVelocity(const std::shared_ptr<Surface>& surface, int panel) const;

    /**
    * @brief
    */
    double pressureCoefficient(const std::shared_ptr<Surface>& surface, int panel) const;

    /**
    * @brief
    */
    Eigen::Vector3d force(const std::shared_ptr<BladeBodyManager>& bladeBody) const;

    /**
    * @brief
    */
    Eigen::Vector3d force(const std::shared_ptr<Surface>& surface) const;

    /**
    * @brief
    */
    Eigen::Vector3d moment(const std::shared_ptr<BladeBodyManager>& bladeBody, const Eigen::Vector3d& x) const;

    /**
    * @brief
    */
    Eigen::Vector3d moment(const std::shared_ptr<Surface>& surface, const Eigen::Vector3d& x) const;

    /**
    * @brief
    */
    std::vector<SurfacePanelPointData, Eigen::aligned_allocator<SurfacePanelPointData> > traceStreamline(const SurfacePanelPointData& start) const;

    /**
    * @brief
    */
    void log(int stepNumber, SurfaceWriter& writer) const;

};