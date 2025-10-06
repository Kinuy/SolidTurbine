#include "LiftingSurfaceSolver.h"

#define VIEW_NAME_SOURCE_DISTRIBUTION   "sigma"
#define VIEW_NAME_DOUBLET_DISTRIBUTION  "mu"
#define VIEW_NAME_PRESSURE_DISTRIBUTION "Cp"
#define VIEW_NAME_VELOCITY_DISTRIBUTION "V"


LiftingSurfaceSolver::LiftingSurfaceSolver(const std::string& logFolder) : logFolder(logFolder)
{
    // Initialize wind:
    freestream_velocity = Eigen::Vector3d(0, 0, 0);

    // Initialize fluid density:
    fluidDensity = 0.0;

    // Total number of panels:
    nonWakePanels = 0;

    // Open log files:
    dirHandler(logFolder);
}

LiftingSurfaceSolver::~LiftingSurfaceSolver()
{
}

void LiftingSurfaceSolver::dirHandler(const std::string& folder) {
    namespace fs = std::filesystem;

    try {
        if (fs::exists(folder)) {
            fs::remove_all(folder); // Delete directory and contents
        }

        if (!fs::create_directory(folder)) {
            std::cerr << "Could not create log folder " << folder << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error handling directory " << folder << ": " << e.what() << std::endl;
    }
}

void LiftingSurfaceSolver::addBody(std::shared_ptr<BladeBodyManager> bladeBody)
{
    static std::shared_ptr<BoundaryLayer> dummy_boundary_layer(new BoundaryLayer());

    addBody(bladeBody, dummy_boundary_layer);
}

void LiftingSurfaceSolver::addBody(std::shared_ptr<BladeBodyManager> bladeBody, std::shared_ptr<BoundaryLayer> boundary_layer)
{
    std::shared_ptr<BladeBodyData> bd(new BladeBodyData(bladeBody, boundary_layer));

    bodies.push_back(bd);

    std::vector<std::shared_ptr<SurfaceData> >::iterator si;
    for (si = bladeBody->nonLiftingSurfaces.begin(); si != bladeBody->nonLiftingSurfaces.end(); si++) {
        std::shared_ptr<SurfaceData> d = *si;

        nonWakeSurfaces.push_back(d);

        surfaceToBody[d->surface] = bd;

        nonWakePanels += d->surface->getNumberOfPanels();
    }

    std::vector<std::shared_ptr<LiftingSurfaceData> >::iterator lsi;
    for (lsi = bladeBody->liftingSurfaces.begin(); lsi != bladeBody->liftingSurfaces.end(); lsi++) {
        std::shared_ptr<LiftingSurfaceData> d = *lsi;

        nonWakeSurfaces.push_back(d);

        surfaceToBody[d->surface] = bd;
        surfaceToBody[d->wake] = bd;

        nonWakePanels += d->liftingSurface->getNumberOfPanels();
    }

    doubletCoefficients.resize(nonWakePanels);
    doubletCoefficients.setZero();

    sourceCoefficients.resize(nonWakePanels);
    sourceCoefficients.setZero();

    surfaceVelocityPotentials.resize(nonWakePanels);
    surfaceVelocityPotentials.setZero();

    surfaceVelocities.resize(nonWakePanels, 3);
    surfaceVelocities.setZero();

    pressureCoefficients.resize(nonWakePanels);
    pressureCoefficients.setZero();

    previousSurfaceVelocityPotentials.resize(nonWakePanels);
    previousSurfaceVelocityPotentials.setZero();

    // Open logs:
    //string body_log_folder = log_folder + "/" + body->id;
    std::string body_log_folder = logFolder + "/" + bladeBody->id;

    dirHandler(body_log_folder);

    for (si = bd->bladeBody->nonLiftingSurfaces.begin(); si != bd->bladeBody->nonLiftingSurfaces.end(); si++) {
        std::shared_ptr<SurfaceData> d = *si;

        std::stringstream ss;
        ss << body_log_folder << "/" << d->surface->id;

        dirHandler(ss.str());
    }

    for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++) {
        std::shared_ptr<LiftingSurfaceData> d = *lsi;

        std::stringstream ss;
        ss << body_log_folder << "/" << d->liftingSurface->id;

        dirHandler(ss.str());

        ss.str(std::string());
        ss.clear();
        ss << body_log_folder << "/" << d->wake->id;

        dirHandler(ss.str());
    }
}

void LiftingSurfaceSolver::setFreestreamVelocity(const Eigen::Vector3d& value)
{
    freestream_velocity = value;
}

void LiftingSurfaceSolver::setFluidDensity(double value)
{
    fluidDensity = value;
}

double LiftingSurfaceSolver::velocityPotential(const Eigen::Vector3d& x) const
{
    return computeVelocityPotential(x);
}

Eigen::Vector3d LiftingSurfaceSolver::velocity(const Eigen::Vector3d& x) const
{
    std::set<int> ignore_set;

    return computeVelocityInterpolated(x, ignore_set);
}

double LiftingSurfaceSolver::surfaceVelocityPotential(const std::shared_ptr<Surface>& surface, int panel) const
{
    int index = computeIndex(surface, panel);
    if (index >= 0)
        return surfaceVelocityPotentials(index);

    std::cerr << "LiftingSurfaceSolver::surfaceVelocityPotential():  Panel " << panel << " not found on surface " << surface->id << "." << std::endl;

    return 0.0;
}

Eigen::Vector3d LiftingSurfaceSolver::surfaceVelocity(const std::shared_ptr<Surface>& surface, int panel) const
{
    int index = computeIndex(surface, panel);
    if (index >= 0)
        return surfaceVelocities.row(index);

    std::cerr << "LiftingSurfaceSolver::surfaceVelocity():  Panel " << panel << " not found on surface " << surface->id << "." << std::endl;

    return Eigen::Vector3d(0, 0, 0);
}

double LiftingSurfaceSolver::pressureCoefficient(const std::shared_ptr<Surface>& surface, int panel) const
{
    int index = computeIndex(surface, panel);
    if (index >= 0)
        return pressureCoefficients(index);

    std::cerr << "LiftingSurfaceSolver::pressureCoefficient():  Panel " << panel << " not found on surface " << surface->id << "." << std::endl;

    return 0.0;
}

Eigen::Vector3d LiftingSurfaceSolver::force(const std::shared_ptr<BladeBodyManager>& body) const
{
    // Dynamic pressure:
    double q = 0.5 * fluidDensity * computeReferenceVelocitySquared(body);

    // Total force on body:
    Eigen::Vector3d F(0, 0, 0);
    int offset = 0;

    std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
    for (si = nonWakeSurfaces.begin(); si != nonWakeSurfaces.end(); si++) {
        const std::shared_ptr<SurfaceData>& d = *si;

        const std::shared_ptr<BladeBodyData>& bd = surfaceToBody.find(d->surface)->second;
        if (body == bd->bladeBody) {
            for (int i = 0; i < d->surface->getNumberOfPanels(); i++) {
                const Eigen::Vector3d& normal = d->surface->panelNormal(i);
                double surface_area = d->surface->panelSurfaceArea(i);
                F += q * surface_area * pressureCoefficients(offset + i) * normal;

                F += bd->boundaryLayer->friction(d->surface, i);
            }
        }

        offset += d->surface->getNumberOfPanels();
    }

    // Done:
    return F;
}

Eigen::Vector3d LiftingSurfaceSolver::force(const std::shared_ptr<Surface>& surface) const
{
    // Total force on surface:
    Eigen::Vector3d F(0, 0, 0);
    int offset = 0;

    std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
    for (si = nonWakeSurfaces.begin(); si != nonWakeSurfaces.end(); si++) {
        const std::shared_ptr<SurfaceData>& d = *si;

        if (d->surface == surface) {
            const std::shared_ptr<BladeBodyData>& bd = surfaceToBody.find(d->surface)->second;

            // Dynamic pressure:
            double q = 0.5 * fluidDensity * computeReferenceVelocitySquared(bd->bladeBody);

            for (int i = 0; i < d->surface->getNumberOfPanels(); i++) {
                const Eigen::Vector3d& normal = d->surface->panelNormal(i);
                double surface_area = d->surface->panelSurfaceArea(i);
                F += q * surface_area * pressureCoefficients(offset + i) * normal;

                F += bd->boundaryLayer->friction(d->surface, i);
            }

            break;
        }

        offset += d->surface->getNumberOfPanels();
    }

    // Done:
    return F;
}

Eigen::Vector3d LiftingSurfaceSolver::moment(const std::shared_ptr<BladeBodyManager>& body, const Eigen::Vector3d& x) const
{
    // Dynamic pressure:
    double q = 0.5 * fluidDensity * computeReferenceVelocitySquared(body);

    // Total moment on body:
    Eigen::Vector3d M(0, 0, 0);
    int offset = 0;

    std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
    for (si = nonWakeSurfaces.begin(); si != nonWakeSurfaces.end(); si++) {
        const std::shared_ptr<SurfaceData>& d = *si;

        const std::shared_ptr<BladeBodyData>& bd = surfaceToBody.find(d->surface)->second;
        if (body == bd->bladeBody) {
            for (int i = 0; i < d->surface->getNumberOfPanels(); i++) {
                const Eigen::Vector3d& normal = d->surface->panelNormal(i);
                double surface_area = d->surface->panelSurfaceArea(i);
                Eigen::Vector3d F = q * surface_area * pressureCoefficients(offset + i) * normal;

                F += bd->boundaryLayer->friction(d->surface, i);

                Eigen::Vector3d r = d->surface->panelCollocationPoint(i, false) - x;
                M += r.cross(F);
            }
        }

        offset += d->surface->getNumberOfPanels();
    }

    // Done:
    return M;
}

Eigen::Vector3d LiftingSurfaceSolver::moment(const std::shared_ptr<Surface>& surface, const Eigen::Vector3d& x) const
{
    // Total moment on surface:
    Eigen::Vector3d M(0, 0, 0);
    int offset = 0;

    std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
    for (si = nonWakeSurfaces.begin(); si != nonWakeSurfaces.end(); si++) {
        const std::shared_ptr<SurfaceData>& d = *si;

        if (d->surface == surface) {
            const std::shared_ptr<BladeBodyData>& bd = surfaceToBody.find(d->surface)->second;

            // Dynamic pressure:
            double q = 0.5 * fluidDensity * computeReferenceVelocitySquared(bd->bladeBody);

            for (int i = 0; i < d->surface->getNumberOfPanels(); i++) {
                const Eigen::Vector3d& normal = d->surface->panelNormal(i);
                double surface_area = d->surface->panelSurfaceArea(i);
                Eigen::Vector3d F = q * surface_area * pressureCoefficients(offset + i) * normal;

                F += bd->boundaryLayer->friction(d->surface, i);

                Eigen::Vector3d r = d->surface->panelCollocationPoint(i, false) - x;
                M += r.cross(F);
            }

            break;
        }

        offset += d->surface->getNumberOfPanels();
    }

    // Done:
    return M;
}

std::vector<SurfacePanelPointData, Eigen::aligned_allocator<SurfacePanelPointData> > LiftingSurfaceSolver::traceStreamline(const SurfacePanelPointData& start) const
{
    std::vector<SurfacePanelPointData, Eigen::aligned_allocator<SurfacePanelPointData> > streamline;

    SurfacePanelPointData cur(start.surface, start.panel, start.point);

    Eigen::Vector3d prev_intersection = start.point;
    int originating_edge = -1;

    // Trace until we hit the end of a surface, or until we hit a stagnation point:
    while (true) {
        // Look up panel velocity:
        Eigen::Vector3d velocity = surfaceVelocity(cur.surface, cur.panel);

        // Stop following the streamline at stagnation points:
        if (velocity.norm() < LiftingSurfaceParameters::zeroThreshold)
            break;

        // Eigen::Transform into a panel frame:
        const Eigen::Transform<double, 3, Eigen::Affine>& transformation = cur.surface->panelCoordinateTransformation(cur.panel);

        Eigen::Vector3d transformed_velocity = transformation.linear() * velocity;
        Eigen::Vector3d transformed_point = transformation * cur.point;

        // Project point onto panel for improved numerical stability:
        transformed_point(2) = 0.0;

        // Intersect with one of the panel edges.
        int edge_id = -1;
        double t = std::numeric_limits<double>::max();
        for (int i = 0; i < (int)cur.surface->panelNodes[cur.panel].size(); i++) {
            // Do not try to intersect with the edge we are already on:
            if (i == originating_edge)
                continue;

            // Compute next node index:
            int next_idx;
            if (i == (int)cur.surface->panelNodes[cur.panel].size() - 1)
                next_idx = 0;
            else
                next_idx = i + 1;

            // Retrieve nodes in panel-local coordinates:  
            const Eigen::Vector3d& node_a = cur.surface->transformedPanelPoints[cur.panel][i];
            const Eigen::Vector3d& node_b = cur.surface->transformedPanelPoints[cur.panel][next_idx];

            // Compute edge:
            Eigen::Vector3d edge = node_b - node_a;

            // Find intersection, if any:
            Eigen::Matrix2d A;
            Eigen::Vector2d b;
            for (int j = 0; j < 2; j++) {
                A(j, 0) = transformed_velocity(j);
                A(j, 1) = -edge(j);
                b(j) = -transformed_point(j) + node_a(j);
            }

            Eigen::ColPivHouseholderQR<Eigen::Matrix2d> solver(A);
            solver.setThreshold(LiftingSurfaceParameters::zeroThreshold);
            if (!solver.isInvertible())
                continue;

            Eigen::Vector2d x = solver.solve(b);

            // Only accept positive quadrant:
            if (x(0) < 0 || x(1) < 0)
                continue;

            // Do not accept infinitesimally small solutions:
            if (x(0) < LiftingSurfaceParameters::zeroThreshold)
                continue;

            // Is this the smallest positive 't' (velocity coefficient)?   
            if (x(0) < t) {
                t = x(0);

                edge_id = i;
            }
        }

        // Dead end?
        if (edge_id < 0)
            break;

        // Compute intersection std::vector:
        Eigen::Vector3d transformed_intersection = transformed_point + t * transformed_velocity;
        Eigen::Vector3d intersection = transformation.inverse() * transformed_intersection;

        // Compute mean between intersection points:
        Eigen::Vector3d mean_point = 0.5 * (intersection + prev_intersection);
        prev_intersection = intersection;

        // Add to streamline:
        SurfacePanelPointData n(cur.surface, cur.panel, mean_point);
        streamline.push_back(n);

        // Find neighbor across edge: 
        // N.B.:  This code assumes that every panel has at most one neighbor across an edge.  
        //        The rest of Vortexje supports more general geometries, however.  This code needs work.
        const std::shared_ptr<BladeBodyData>& bd = surfaceToBody.find(cur.surface)->second;
        std::vector<SurfacePanelEdgeData> neighbors = bd->bladeBody->panelNeighbors(cur.surface, cur.panel, edge_id);

        // No neighbor?
        if (neighbors.size() == 0)
            break;

        // Verify the direction of the neighboring velocity std::vector:
        Eigen::Vector3d neighbor_velocity = surfaceVelocity(neighbors[0].surface, neighbors[0].panel);

        const Eigen::Vector3d& normal = cur.surface->panelNormal(cur.panel);
        const Eigen::Vector3d& neighbor_normal = neighbors[0].surface->panelNormal(neighbors[0].panel);

        Eigen::Quaterniond unfold = Eigen::Quaterniond::FromTwoVectors(neighbor_normal, normal);
        Eigen::Vector3d unfolded_neighbor_velocity = unfold * neighbor_velocity;

        if (velocity.dot(unfolded_neighbor_velocity) < 0) {
            // Velocity vectors point in opposite directions.
            break;
        }

        // Proceed to neighboring panel:
        cur.surface = neighbors[0].surface;
        cur.panel = neighbors[0].panel;
        cur.point = intersection;

        originating_edge = neighbors[0].edge;
    }

    // Done:
    return streamline;
}

void LiftingSurfaceSolver::initializeWakes(double dt)
{
    // Add initial wake layers:
    std::vector<std::shared_ptr<BladeBodyData> >::iterator bdi;
    for (bdi = bodies.begin(); bdi != bodies.end(); bdi++) {
        std::shared_ptr<BladeBodyData> bd = *bdi;

        std::vector<std::shared_ptr<LiftingSurfaceData> >::iterator lsi;
        for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++) {
            std::shared_ptr<LiftingSurfaceData> d = *lsi;

            d->wake->addLayer();
            for (int i = 0; i < d->liftingSurface->getSpanwiseNodesCount(); i++) {
                if (LiftingSurfaceParameters::convectWake) {
                    // Convect wake nodes that coincide with the trailing edge.
                    d->wake->nodes[i] += computeTrailingEdgeVortexDisplacement(bd->bladeBody, d->liftingSurface, i, dt);

                }
                else {
                    // Initialize static wake->
                    Eigen::Vector3d body_apparent_velocity = bd->bladeBody->velocity - freestream_velocity;

                    d->wake->nodes[i] -= LiftingSurfaceParameters::staticWakeLength * body_apparent_velocity / body_apparent_velocity.norm();
                }
            }

            d->wake->addLayer();
        }
    }
}

bool LiftingSurfaceSolver::solve(double dt, bool propagate)
{
    int offset;

    // Iterate inviscid and boundary layer solutions until convergence.
    Eigen::VectorXd previous_source_coefficients;
    Eigen::VectorXd previous_doublet_coefficients;

    int boundary_layer_iteration = 0;

    while (true) {
        // Copy state:
        previous_source_coefficients = sourceCoefficients;
        previous_doublet_coefficients = doubletCoefficients;

        // Compute new source distribution:
        std::cout << "LiftingSurfaceSolver: Computing source distribution with wake influence." << std::endl;

        offset = 0;

        std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
        for (si = nonWakeSurfaces.begin(); si != nonWakeSurfaces.end(); si++) {
            const std::shared_ptr<SurfaceData>& d = *si;
            int i;

            const std::shared_ptr<BladeBodyData>& bd = surfaceToBody.find(d->surface)->second;

#pragma omp parallel
            {
#pragma omp for schedule(dynamic, 1)
                for (i = 0; i < d->surface->getNumberOfPanels(); i++)
                    sourceCoefficients(offset + i) = computeSourceCoefficient(bd->bladeBody, d->surface, i, bd->boundaryLayer, true);
            }

            offset += d->surface->getNumberOfPanels();
        }

        // Populate the matrices of influence coefficients:
        std::cout << "LiftingSurfaceSolver: Computing matrices of influence coefficients." << std::endl;

        Eigen::MatrixXd A(nonWakePanels, nonWakePanels);
        Eigen::MatrixXd source_influence_coefficients(nonWakePanels, nonWakePanels);

        int offset_row = 0, offset_col = 0;

        std::vector<std::shared_ptr<SurfaceData> >::const_iterator si_row;
        for (si_row = nonWakeSurfaces.begin(); si_row != nonWakeSurfaces.end(); si_row++) {
            const std::shared_ptr<SurfaceData>& d_row = *si_row;

            offset_col = 0;

            // Influence coefficients between all non-wake surfaces:
            std::vector<std::shared_ptr<SurfaceData> >::const_iterator si_col;
            for (si_col = nonWakeSurfaces.begin(); si_col != nonWakeSurfaces.end(); si_col++) {
                std::shared_ptr<SurfaceData> d_col = *si_col;
                int i, j;

#pragma omp parallel private(j) 
                {
#pragma omp for schedule(dynamic, 1)
                    for (i = 0; i < d_row->surface->getNumberOfPanels(); i++) {
                        for (j = 0; j < d_col->surface->getNumberOfPanels(); j++) {
                            d_col->surface->sourceAndDoubletInfluence(d_row->surface, i, j,
                                source_influence_coefficients(offset_row + i, offset_col + j),
                                A(offset_row + i, offset_col + j));
                        }
                    }
                }

                offset_col = offset_col + d_col->surface->getNumberOfPanels();
            }

            // The influence of the new wake panels:
            int i, j, lifting_surface_offset, wake_panel_offset, pa, pb;
            std::vector<std::shared_ptr<BladeBodyData> >::const_iterator bdi;
            std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
            std::vector<std::shared_ptr<LiftingSurfaceData> >::const_iterator lsi;
            std::shared_ptr<BladeBodyData> bd;
            std::shared_ptr<LiftingSurfaceData> d;

#pragma omp parallel private(bdi, si, lsi, lifting_surface_offset, j, wake_panel_offset, pa, pb, bd, d)
            {
#pragma omp for schedule(dynamic, 1)
                for (i = 0; i < d_row->surface->getNumberOfPanels(); i++) {
                    lifting_surface_offset = 0;

                    for (bdi = bodies.begin(); bdi != bodies.end(); bdi++) {
                        bd = *bdi;

                        for (si = bd->bladeBody->nonLiftingSurfaces.begin(); si != bd->bladeBody->nonLiftingSurfaces.end(); si++)
                            lifting_surface_offset += (*si)->surface->getNumberOfPanels();

                        for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++) {
                            d = *lsi;

                            wake_panel_offset = d->wake->getNumberOfPanels() - d->liftingSurface->getSpanwisePanelsCount();
                            for (j = 0; j < d->liftingSurface->getSpanwisePanelsCount(); j++) {
                                pa = d->liftingSurface->trailingEdgeTopPanel(j);
                                pb = d->liftingSurface->trailingEdgeBottomPanel(j);

                                // Account for the influence of the new wake panels.  The doublet strength of these panels
                                // is std::set according to the Kutta condition.
                                A(offset_row + i, lifting_surface_offset + pa) += d->wake->doubletInfluence(d_row->surface, i, wake_panel_offset + j);
                                A(offset_row + i, lifting_surface_offset + pb) -= d->wake->doubletInfluence(d_row->surface, i, wake_panel_offset + j);
                            }

                            lifting_surface_offset += d->liftingSurface->getNumberOfPanels();
                        }
                    }
                }
            }

            offset_row = offset_row + d_row->surface->getNumberOfPanels();
        }

        // Compute new doublet distribution:
        std::cout << "LiftingSurfaceSolver: Computing doublet distribution." << std::endl;

        Eigen::VectorXd b = source_influence_coefficients * sourceCoefficients;

        Eigen::BiCGSTAB<Eigen::MatrixXd, Eigen::DiagonalPreconditioner<double> > solver(A);
        solver.setMaxIterations(LiftingSurfaceParameters::linearSolverMaxIterations);
        solver.setTolerance(LiftingSurfaceParameters::linearSolverTolerance);

        doubletCoefficients = solver.solveWithGuess(b, previous_doublet_coefficients);

        if (solver.info() != Eigen::Success) {
            std::cerr << "LiftingSurfaceSolver: Computing doublet distribution failed (" << solver.iterations();
            std::cerr << " iterations with estimated error=" << solver.error() << ")." << std::endl;

            return false;
        }

        std::cout << "LiftingSurfaceSolver: Done computing doublet distribution in " << solver.iterations() << " iterations with estimated error " << solver.error() << "." << std::endl;

        // Check for convergence from second iteration onwards.
        bool converged = false;
        if (boundary_layer_iteration > 0) {
            double delta = (sourceCoefficients - previous_source_coefficients).norm();

            std::cout << "LiftingSurfaceSolver: Boundary layer convergence delta = " << delta << std::endl;

            if (delta < LiftingSurfaceParameters::rlBoundaryLayerIterationTolerance)
                converged = true;
        }

        // Set new wake panel doublet coefficients:
        std::cout << "LiftingSurfaceSolver: Updating wake doublet distribution." << std::endl;

        offset = 0;

        std::vector<std::shared_ptr<BladeBodyData> >::iterator bdi;
        for (bdi = bodies.begin(); bdi != bodies.end(); bdi++) {
            std::shared_ptr<BladeBodyData> bd = *bdi;

            std::vector<std::shared_ptr<SurfaceData> >::iterator si;
            for (si = bd->bladeBody->nonLiftingSurfaces.begin(); si != bd->bladeBody->nonLiftingSurfaces.end(); si++)
                offset += (*si)->surface->getNumberOfPanels();

            std::vector<std::shared_ptr<LiftingSurfaceData> >::iterator lsi;
            for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++) {
                std::shared_ptr<LiftingSurfaceData> d = *lsi;

                // Set panel doublet coefficient:
                for (int i = 0; i < d->liftingSurface->getSpanwisePanelsCount(); i++) {
                    double doublet_coefficient_top = doubletCoefficients(offset + d->liftingSurface->trailingEdgeTopPanel(i));
                    double doublet_coefficient_bottom = doubletCoefficients(offset + d->liftingSurface->trailingEdgeBottomPanel(i));

                    // Use the trailing-edge Kutta condition to compute the doublet coefficients of the new wake panels.
                    double doublet_coefficient = doublet_coefficient_top - doublet_coefficient_bottom;

                    int idx = d->wake->getNumberOfPanels() - d->liftingSurface->getSpanwisePanelsCount() + i;
                    d->wake->doubletCoefficients[idx] = doublet_coefficient;
                }

                // Update offset:
                offset += d->liftingSurface->getNumberOfPanels();
            }
        }

        // Compute surface velocity distribution:
        std::cout << "LiftingSurfaceSolver: Computing surface velocity distribution." << std::endl;

        offset = 0;

        for (bdi = bodies.begin(); bdi != bodies.end(); bdi++) {
            std::shared_ptr<BladeBodyData> bd = *bdi;

            std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
            for (si = bd->bladeBody->nonLiftingSurfaces.begin(); si != bd->bladeBody->nonLiftingSurfaces.end(); si++) {
                const std::shared_ptr<SurfaceData>& d = *si;
                int i;

#pragma omp parallel
                {
#pragma omp for schedule(dynamic, 1)
                    for (i = 0; i < d->surface->getNumberOfPanels(); i++)
                        surfaceVelocities.row(offset + i) = computeSurfaceVelocity(bd->bladeBody, d->surface, i);
                }

                offset += d->surface->getNumberOfPanels();
            }

            std::vector<std::shared_ptr<LiftingSurfaceData> >::const_iterator lsi;
            for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++) {
                const std::shared_ptr<LiftingSurfaceData>& d = *lsi;
                int i;

#pragma omp parallel
                {
#pragma omp for schedule(dynamic, 1)
                    for (i = 0; i < d->surface->getNumberOfPanels(); i++)
                        surfaceVelocities.row(offset + i) = computeSurfaceVelocity(bd->bladeBody, d->surface, i);
                }

                offset += d->surface->getNumberOfPanels();
            }
        }

        // If we converged, then this is the time to break out of the loop.
        if (converged) {
            std::cout << "LiftingSurfaceSolver: Boundary layer iteration converged in " << boundary_layer_iteration << " steps." << std::endl;

            break;
        }

        if (boundary_layer_iteration > LiftingSurfaceParameters::rlMaxBoundaryLayerIterations) {
            std::cout << "LiftingSurfaceSolver: Maximum number of boundary layer iterations ranged.  Aborting iteration." << std::endl;

            break;
        }

        // Recompute the boundary layers.
        offset = 0;

        bool have_boundary_layer = false;
        for (bdi = bodies.begin(); bdi != bodies.end(); bdi++) {
            std::shared_ptr<BladeBodyData> bd = *bdi;

            // Count panels on body:
            int body_n_panels = 0;

            std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
            for (si = bd->bladeBody->nonLiftingSurfaces.begin(); si != bd->bladeBody->nonLiftingSurfaces.end(); si++) {
                const std::shared_ptr<SurfaceData>& d = *si;

                body_n_panels += d->surface->getNumberOfPanels();
            }

            std::vector<std::shared_ptr<LiftingSurfaceData> >::const_iterator lsi;
            for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++) {
                const std::shared_ptr<LiftingSurfaceData>& d = *lsi;

                body_n_panels += d->surface->getNumberOfPanels();
            }

            // Recompute boundary layer:
            if (typeid(*bd->boundaryLayer.get()) != typeid(BoundaryLayer)) {
                have_boundary_layer = true;

                if (!bd->boundaryLayer->recalculate(freestream_velocity, surfaceVelocities.block(offset, 0, body_n_panels, 3)))
                    return false;
            }

            offset += body_n_panels;
        }

        // Did we did not find any boundary layers, then there is no need to iterate.
        if (!have_boundary_layer)
            break;

        // Increase iteration counter:
        boundary_layer_iteration++;
    }

    if (LiftingSurfaceParameters::convectWake) {
        // Recompute source distribution without wake influence:
        std::cout << "LiftingSurfaceSolver: Recomputing source distribution without wake influence." << std::endl;

        offset = 0;

        std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
        for (si = nonWakeSurfaces.begin(); si != nonWakeSurfaces.end(); si++) {
            const std::shared_ptr<SurfaceData>& d = *si;
            int i;

            const std::shared_ptr<BladeBodyData>& bd = surfaceToBody.find(d->surface)->second;

#pragma omp parallel
            {
#pragma omp for schedule(dynamic, 1)
                for (i = 0; i < d->surface->getNumberOfPanels(); i++)
                    sourceCoefficients(offset + i) = computeSourceCoefficient(bd->bladeBody, d->surface, i, bd->boundaryLayer, false);
            }

            offset += d->surface->getNumberOfPanels();
        }
    }

    // Compute pressure distribution:
    std::cout << "LiftingSurfaceSolver: Computing pressure distribution." << std::endl;

    offset = 0;

    std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
    for (si = nonWakeSurfaces.begin(); si != nonWakeSurfaces.end(); si++) {
        const std::shared_ptr<SurfaceData>& d = *si;
        int i;

        const std::shared_ptr<BladeBodyData>& bd = surfaceToBody.find(d->surface)->second;
        double v_ref_squared = computeReferenceVelocitySquared(bd->bladeBody);

        double dphidt;

#pragma omp parallel private(dphidt)
        {
#pragma omp for schedule(dynamic, 1)
            for (i = 0; i < d->surface->getNumberOfPanels(); i++) {
                // Velocity potential:
                surfaceVelocityPotentials(offset + i) = computeSurfaceVelocityPotential(d->surface, offset, i);

                // Pressure coefficient:
                dphidt = computeSurfaceVelocityPotentialTimeDerivative(offset, i, dt);
                pressureCoefficients(offset + i) = computePressureCoefficient(surfaceVelocities.row(offset + i), dphidt, v_ref_squared);
            }
        }

        offset += d->surface->getNumberOfPanels();
    }

    // Propagate solution forward in time, if requested.
    if (propagate)
        this->propagate();

    // Done:
    return true;
}

void LiftingSurfaceSolver::propagate()
{
    // Store previous values of the surface velocity potentials:
    previousSurfaceVelocityPotentials = surfaceVelocityPotentials;
}

void LiftingSurfaceSolver::updateWakes(double dt)
{
    // Do we convect wake panels?
    if (LiftingSurfaceParameters::convectWake) {
        std::cout << "LiftingSurfaceSolver: Convecting wakes." << std::endl;

        // Compute velocity values at wake nodes, with the wakes in their original state:
        using AlignedVector3d = std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d>>;
        using AlignedMatrix3d = std::vector<AlignedVector3d, Eigen::aligned_allocator<AlignedVector3d>>;
        AlignedMatrix3d wake_velocities;

        //std::vector<Eigen::Matrix3Xd, Eigen::aligned_allocator<Eigen::Matrix3Xd>> wake_velocities;

        //std::vector<std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> >, Eigen::aligned_allocator<std::vector<Eigen::Vector3d> > > wake_velocities;

        std::vector<std::shared_ptr<BladeBodyData> >::const_iterator bdi;
        for (bdi = bodies.begin(); bdi != bodies.end(); bdi++) {
            const std::shared_ptr<BladeBodyData>& bd = *bdi;

            std::vector<std::shared_ptr<LiftingSurfaceData> >::const_iterator lsi;
            for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++) {
                const std::shared_ptr<LiftingSurfaceData>& d = *lsi;

                std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > local_wake_velocities;
                local_wake_velocities.resize(d->wake->getNumberOfNodes());

                int i;

#pragma omp parallel
                {
#pragma omp for schedule(dynamic, 1)
                    for (i = 0; i < d->wake->getNumberOfNodes(); i++)
                        local_wake_velocities[i] = velocity(d->wake->nodes[i]);
                }

                wake_velocities.push_back(local_wake_velocities);
            }
        }

        // Add new wake panels at trailing edges, and convect all vertices:
        int idx = 0;

        for (bdi = bodies.begin(); bdi != bodies.end(); bdi++) {
            std::shared_ptr<BladeBodyData> bd = *bdi;

            std::vector<std::shared_ptr<LiftingSurfaceData> >::iterator lsi;
            for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++) {
                std::shared_ptr<LiftingSurfaceData> d = *lsi;

                // Retrieve local wake velocities:
                std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> >& local_wake_velocities = wake_velocities[idx];
                idx++;

                // Convect wake nodes that coincide with the trailing edge.
                for (int i = 0; i < d->liftingSurface->getSpanwiseNodesCount(); i++) {
                    d->wake->nodes[d->wake->getNumberOfNodes() - d->liftingSurface->getSpanwiseNodesCount() + i]
                        += computeTrailingEdgeVortexDisplacement(bd->bladeBody, d->liftingSurface, i, dt);
                }

                // Convect all other wake nodes according to the local wake velocity:
                int i;

#pragma omp parallel
                {
#pragma omp for schedule(dynamic, 1)
                    for (i = 0; i < d->wake->getNumberOfNodes() - d->liftingSurface->getSpanwiseNodesCount(); i++)
                        d->wake->nodes[i] += local_wake_velocities[i] * dt;
                }

                // Run internal wake update:
                d->wake->updateProperties(dt);

                // Add new vertices:
                // (This call also updates the geometry)
                d->wake->addLayer();
            }
        }

    }
    else {
        std::cout << "LiftingSurfaceSolver: Re-positioning wakes." << std::endl;

        // No wake convection.  Re-position wake:
        std::vector<std::shared_ptr<BladeBodyData> >::iterator bdi;
        for (bdi = bodies.begin(); bdi != bodies.end(); bdi++) {
            std::shared_ptr<BladeBodyData> bd = *bdi;

            Eigen::Vector3d body_apparent_velocity = bd->bladeBody->velocity - freestream_velocity;

            std::vector<std::shared_ptr<LiftingSurfaceData> >::iterator lsi;
            for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++) {
                std::shared_ptr<LiftingSurfaceData> d = *lsi;

                for (int i = 0; i < d->liftingSurface->getSpanwiseNodesCount(); i++) {
                    // Connect wake to trailing edge nodes:                             
                    d->wake->nodes[d->liftingSurface->getSpanwiseNodesCount() + i] = d->liftingSurface->nodes[d->liftingSurface->trailingEdgeNode(i)];

                    // Point wake in direction of body kinematic velocity:
                    d->wake->nodes[i] = d->liftingSurface->nodes[d->liftingSurface->trailingEdgeNode(i)]
                        - LiftingSurfaceParameters::staticWakeLength * body_apparent_velocity / body_apparent_velocity.norm();
                }

                // Need to update geometry:
                d->wake->calcGeometry();
            }
        }
    }
}

void LiftingSurfaceSolver::log(int stepNumber, SurfaceWriter& writer) const
{
    // Log coefficients: 
    int offset = 0;
    int save_node_offset = 0;
    int save_panel_offset = 0;
    int idx;

    std::vector<std::shared_ptr<BladeBodyData> >::const_iterator bdi;
    for (bdi = bodies.begin(); bdi != bodies.end(); bdi++) {
        const std::shared_ptr<BladeBodyData>& bd = *bdi;

        // Iterate non-lifting surfaces:
        idx = 0;

        std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
        for (si = bd->bladeBody->nonLiftingSurfaces.begin(); si != bd->bladeBody->nonLiftingSurfaces.end(); si++) {
            const std::shared_ptr<SurfaceData>& d = *si;

            // Log non-lifting surface coefficients:
            Eigen::MatrixXd non_lifting_surface_doublet_coefficients(d->surface->getNumberOfPanels(), 1);
            Eigen::MatrixXd non_lifting_surface_source_coefficients(d->surface->getNumberOfPanels(), 1);
            Eigen::MatrixXd non_lifting_surface_pressure_coefficients(d->surface->getNumberOfPanels(), 1);
            Eigen::MatrixXd non_lifting_surface_velocity_vectors(d->surface->getNumberOfPanels(), 3);
            for (int i = 0; i < d->surface->getNumberOfPanels(); i++) {
                non_lifting_surface_doublet_coefficients(i, 0) = doubletCoefficients(offset + i);
                non_lifting_surface_source_coefficients(i, 0) = sourceCoefficients(offset + i);
                non_lifting_surface_pressure_coefficients(i, 0) = pressureCoefficients(offset + i);
                non_lifting_surface_velocity_vectors.row(i) = surfaceVelocities.row(offset + i);
            }

            offset += d->surface->getNumberOfPanels();

            std::vector<std::string> view_names;
            std::vector<Eigen::MatrixXd, Eigen::aligned_allocator<Eigen::MatrixXd> > view_data;

            view_names.push_back(VIEW_NAME_DOUBLET_DISTRIBUTION);
            view_data.push_back(non_lifting_surface_doublet_coefficients);

            view_names.push_back(VIEW_NAME_SOURCE_DISTRIBUTION);
            view_data.push_back(non_lifting_surface_source_coefficients);

            view_names.push_back(VIEW_NAME_PRESSURE_DISTRIBUTION);
            view_data.push_back(non_lifting_surface_pressure_coefficients);

            view_names.push_back(VIEW_NAME_VELOCITY_DISTRIBUTION);
            view_data.push_back(non_lifting_surface_velocity_vectors);

            std::stringstream ss;
            ss << logFolder << "/" << bd->bladeBody->id << "/" << d->surface->id << "/step_" << stepNumber << writer.fileExtension();

            writer.write(d->surface, ss.str(), save_node_offset, save_panel_offset, view_names, view_data);

            save_node_offset += d->surface->getNumberOfNodes();
            save_panel_offset += d->surface->getNumberOfPanels();

            idx++;
        }

        // Iterate lifting surfaces:
        idx = 0;

        std::vector<std::shared_ptr<LiftingSurfaceData> >::const_iterator lsi;
        for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++) {
            const std::shared_ptr<LiftingSurfaceData>& d = *lsi;

            // Log lifting surface coefficients:
            Eigen::MatrixXd lifting_surface_doublet_coefficients(d->liftingSurface->getNumberOfPanels(), 1);
            Eigen::MatrixXd lifting_surface_source_coefficients(d->liftingSurface->getNumberOfPanels(), 1);
            Eigen::MatrixXd lifting_surface_pressure_coefficients(d->liftingSurface->getNumberOfPanels(), 1);
            Eigen::MatrixXd lifting_surface_velocity_vectors(d->surface->getNumberOfPanels(), 3);
            for (int i = 0; i < d->liftingSurface->getNumberOfPanels(); i++) {
                lifting_surface_doublet_coefficients(i, 0) = doubletCoefficients(offset + i);
                lifting_surface_source_coefficients(i, 0) = sourceCoefficients(offset + i);
                lifting_surface_pressure_coefficients(i, 0) = pressureCoefficients(offset + i);
                lifting_surface_velocity_vectors.row(i) = surfaceVelocities.row(offset + i);
            }

            offset += d->liftingSurface->getNumberOfPanels();

            std::vector<std::string> view_names;
            std::vector<Eigen::MatrixXd, Eigen::aligned_allocator<Eigen::MatrixXd> > view_data;

            view_names.push_back(VIEW_NAME_DOUBLET_DISTRIBUTION);
            view_data.push_back(lifting_surface_doublet_coefficients);

            view_names.push_back(VIEW_NAME_SOURCE_DISTRIBUTION);
            view_data.push_back(lifting_surface_source_coefficients);

            view_names.push_back(VIEW_NAME_PRESSURE_DISTRIBUTION);
            view_data.push_back(lifting_surface_pressure_coefficients);

            view_names.push_back(VIEW_NAME_VELOCITY_DISTRIBUTION);
            view_data.push_back(lifting_surface_velocity_vectors);

            std::stringstream ss;
            ss << logFolder << "/" << bd->bladeBody->id << "/" << d->liftingSurface->id << "/step_" << stepNumber << writer.fileExtension();

            writer.write(d->liftingSurface, ss.str(), save_node_offset, save_panel_offset, view_names, view_data);

            save_node_offset += d->liftingSurface->getNumberOfNodes();
            save_panel_offset += d->liftingSurface->getNumberOfPanels();

            // Log wake surface and coefficients:
            Eigen::MatrixXd wake_doublet_coefficients(d->wake->doubletCoefficients.size(), 1);
            for (int i = 0; i < (int)d->wake->doubletCoefficients.size(); i++)
                wake_doublet_coefficients(i, 0) = d->wake->doubletCoefficients[i];

            view_names.clear();
            view_data.clear();

            view_names.push_back(VIEW_NAME_DOUBLET_DISTRIBUTION);
            view_data.push_back(wake_doublet_coefficients);

            std::stringstream ssw;
            ssw << logFolder << "/" << bd->bladeBody->id << "/" << d->wake->id << "/step_" << stepNumber << writer.fileExtension();

            writer.write(d->wake, ssw.str(), 0, save_panel_offset, view_names, view_data);

            save_node_offset += d->wake->getNumberOfNodes();
            save_panel_offset += d->wake->getNumberOfPanels();

            idx++;
        }
    }
}

double LiftingSurfaceSolver::computeSourceCoefficient(const std::shared_ptr<BladeBodyManager>& body, const std::shared_ptr<Surface>& surface, int panel, const std::shared_ptr<BoundaryLayer>& boundaryLayer, bool includeWakeInfluence) const
{
    // Start with apparent velocity:
    Eigen::Vector3d velocity = body->panelKinematicVelocity(surface, panel) - freestream_velocity;

    // Wake contribution:
    if (LiftingSurfaceParameters::convectWake && includeWakeInfluence) {
        std::vector<std::shared_ptr<BladeBodyData> >::const_iterator bdi;
        for (bdi = bodies.begin(); bdi != bodies.end(); bdi++) {
            const std::shared_ptr<BladeBodyData>& bd = *bdi;

            std::vector<std::shared_ptr<LiftingSurfaceData> >::const_iterator lsi;
            for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++) {
                const std::shared_ptr<LiftingSurfaceData>& d = *lsi;

                // Add influence of old wake panels.  That is, those wake panels which already have a doublet
                // strength assigned to them.
                for (int k = 0; k < d->wake->getNumberOfPanels() - d->liftingSurface->getSpanwisePanelsCount(); k++) {
                    // Use doublet panel - vortex ring equivalence.
                    velocity -= d->wake->vortexRingUnitVelocity(surface->panelCollocationPoint(panel, true), k)
                        * d->wake->doubletCoefficients[k];
                }
            }
        }
    }

    // Take normal component, and subtract blowing velocity:
    const Eigen::Vector3d& normal = surface->panelNormal(panel);

    double blowingVelocity = boundaryLayer->transpirationVelocity(surface, panel);

    return velocity.dot(normal) - blowingVelocity;
}

double LiftingSurfaceSolver::computeSurfaceVelocityPotential(const std::shared_ptr<Surface>& surface, int offset, int panel) const
{
    double phi = -doubletCoefficients(offset + panel);

    // Add flow potential due to kinematic velocity:
    const std::shared_ptr<BladeBodyData>& bd = surfaceToBody.find(surface)->second;
    Eigen::Vector3d apparent_velocity = bd->bladeBody->panelKinematicVelocity(surface, panel) - freestream_velocity;

    phi -= apparent_velocity.dot(surface->panelCollocationPoint(panel, false));

    return phi;
}

double LiftingSurfaceSolver::computeSurfaceVelocityPotentialTimeDerivative(int offset, int panel, double dt) const
{
    double dphidt;

    // Evaluate the time-derivative of the potential in a body-fixed reference frame, as in
    //   J. P. Giesing, Nonlinear Two-Dimensional Unsteady Potential Flow with Lift, Journal of Aircraft, 1968.
    if (LiftingSurfaceParameters::unsteadyBernoulli && dt > 0.0)
        dphidt = (surfaceVelocityPotentials(offset + panel) - previousSurfaceVelocityPotentials(offset + panel)) / dt;
    else
        dphidt = 0.0;

    return dphidt;
}

Eigen::Vector3d LiftingSurfaceSolver::computeScalarFieldGradient(const Eigen::VectorXd& scalarField, const std::shared_ptr<BladeBodyManager>& bladeBody, const std::shared_ptr<Surface>& surface, int panel) const
{
    // We compute the scalar field gradient by fitting a linear model.

    // Retrieve panel neighbors.
    std::vector<SurfacePanelEdgeData> neighbors = bladeBody->panelNeighbors(surface, panel);

    // Set up a transformation such that panel normal becomes unit Z std::vector:
    Eigen::Transform<double, 3, Eigen::Affine> transformation = surface->panelCoordinateTransformation(panel);

    // Set up model equations:
    Eigen::MatrixXd A(neighbors.size(), 2);
    Eigen::VectorXd b(neighbors.size());

    // The model is centered on panel:
    double panel_value = scalarField(computeIndex(surface, panel));

    for (int i = 0; i < (int)neighbors.size(); i++) {
        SurfacePanelEdgeData neighbor_panel = neighbors[i];

        // Add neighbor relative to panel:
        Eigen::Vector3d neighbor_vector_normalized = transformation * neighbor_panel.surface->panelCollocationPoint(neighbor_panel.panel, false);

        A(i, 0) = neighbor_vector_normalized(0);
        A(i, 1) = neighbor_vector_normalized(1);

        b(i) = scalarField(computeIndex(neighbor_panel.surface, neighbor_panel.panel)) - panel_value;
    }

    // Solve model equations:
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(A, Eigen::ComputeThinU | Eigen::ComputeThinV);
    svd.setThreshold(LiftingSurfaceParameters::zeroThreshold);

    Eigen::VectorXd model_coefficients = svd.solve(b);

    // Extract gradient in local frame:
    Eigen::Vector3d gradient_normalized = Eigen::Vector3d(model_coefficients(0), model_coefficients(1), 0.0);

    // Eigen::Transform gradient to global frame:
    return transformation.linear().transpose() * gradient_normalized;
}

Eigen::Vector3d LiftingSurfaceSolver::computeSurfaceVelocity(const std::shared_ptr<BladeBodyManager>& bladeBody, const std::shared_ptr<Surface>& surface, int panel) const
{
    // Compute doublet surface gradient:
    Eigen::Vector3d tangential_velocity = -computeScalarFieldGradient(doubletCoefficients, bladeBody, surface, panel);

    // Add flow due to kinematic velocity:
    Eigen::Vector3d apparent_velocity = bladeBody->panelKinematicVelocity(surface, panel) - freestream_velocity;

    tangential_velocity -= apparent_velocity;

    // Remove any normal velocity.  This is the (implicit) contribution of the source term.
    const Eigen::Vector3d& normal = surface->panelNormal(panel);
    tangential_velocity -= tangential_velocity.dot(normal) * normal;

    // Done:
    return tangential_velocity;
}

double LiftingSurfaceSolver::computeReferenceVelocitySquared(const std::shared_ptr<BladeBodyManager>& bladeBody) const
{
    return (bladeBody->velocity - freestream_velocity).squaredNorm();
}

double LiftingSurfaceSolver::computePressureCoefficient(const Eigen::Vector3d& surfaceVelocity, double dphidt, double v_ref_squared) const
{
    double C_p = 1 - (surfaceVelocity.squaredNorm() + 2 * dphidt) / v_ref_squared;

    return C_p;
}

double LiftingSurfaceSolver::computeVelocityPotential(const Eigen::Vector3d& x) const
{
    double phi = 0.0;

    // Iterate all non-wake surfaces:
    int offset = 0;

    std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
    for (si = nonWakeSurfaces.begin(); si != nonWakeSurfaces.end(); si++) {
        const std::shared_ptr<SurfaceData>& d = *si;

        for (int i = 0; i < d->surface->getNumberOfPanels(); i++) {
            double sourceInfluence, doubletInfluence;

            d->surface->sourceAndDoubletInfluence(x, i, sourceInfluence, doubletInfluence);

            phi += doubletInfluence * doubletCoefficients(offset + i);
            phi += sourceInfluence * sourceCoefficients(offset + i);
        }

        offset += d->surface->getNumberOfPanels();
    }

    // Iterate wakes:
    std::vector<std::shared_ptr<BladeBodyData> >::const_iterator bdi;
    for (bdi = bodies.begin(); bdi != bodies.end(); bdi++) {
        const std::shared_ptr<BladeBodyData>& bd = *bdi;

        std::vector<std::shared_ptr<LiftingSurfaceData> >::const_iterator lsi;
        for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++) {
            const std::shared_ptr<LiftingSurfaceData>& d = *lsi;

            for (int i = 0; i < d->wake->getNumberOfPanels(); i++)
                phi += d->wake->doubletInfluence(x, i) * d->wake->doubletCoefficients[i];
        }
    }

    // Done:
    return phi + freestream_velocity.dot(x);
}

Eigen::Vector3d LiftingSurfaceSolver::computeVelocityInterpolated(const Eigen::Vector3d& x, std::set<int>& ignoreSet) const
{
    // Lists of close velocities, ordered by primacy:
    std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > close_panel_velocities;
    std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > close_panel_edge_velocities;
    std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > interior_close_panel_velocities;
    std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > interior_close_panel_edge_velocities;

    std::vector<std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> >* > velocity_lists;
    velocity_lists.push_back(&close_panel_velocities);
    velocity_lists.push_back(&close_panel_edge_velocities);
    velocity_lists.push_back(&interior_close_panel_velocities);
    velocity_lists.push_back(&interior_close_panel_edge_velocities);

    // Iterate bodies:
    int offset = 0;

    std::vector<std::shared_ptr<BladeBodyData> >::const_iterator bdi;
    for (bdi = bodies.begin(); bdi != bodies.end(); bdi++) {
        const std::shared_ptr<BladeBodyData>& bd = *bdi;

        // Iterate surfaces:
        std::vector<std::shared_ptr<SurfaceData> > surfaces;
        std::vector<std::shared_ptr<LiftingSurfaceData> >::const_iterator lsi;
        std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
        for (si = bd->bladeBody->nonLiftingSurfaces.begin(); si != bd->bladeBody->nonLiftingSurfaces.end(); si++)
            surfaces.push_back(*si);
        for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++)
            surfaces.push_back(*lsi);

        for (si = surfaces.begin(); si != surfaces.end(); si++) {
            const std::shared_ptr<SurfaceData>& d = *si;

            for (int i = 0; i < d->surface->getNumberOfPanels(); i++) {
                // Ignore the given std::set of panels.
                if (ignoreSet.find(i) != ignoreSet.end())
                    continue;

                // Eigen::Transform the point 'x' into the panel coordinate system:
                Eigen::Vector3d x_transformed = d->surface->panelCoordinateTransformation(i) * x;

                // Are we in the exterior, relative to the panel?
                bool in_exterior;
                if (x_transformed(2) < LiftingSurfaceParameters::zeroThreshold)
                    in_exterior = true;
                else
                    in_exterior = false;

                // Compute normal distance of the point 'x' from panel:
                double normal_distance = fabs(x_transformed(2));

                // We have three zones: 
                // The boundary layer, followed by the interpolation layer, followed by the rest of the control volume.
                double boundary_layer_thickness = bd->boundaryLayer->thickness(d->surface, i);
                double interpolation_layer_thickness = LiftingSurfaceParameters::rlInterpolationLayerThickness;
                double total_thickness = boundary_layer_thickness + interpolation_layer_thickness;

                // Are we inside one of the first two layers?
                if (normal_distance < total_thickness) {
                    // Yes.  Check whether A) the point projection lies inside the panel, and whether B) we are close to one of the panel's edges:
                    bool projection_in_panel = true;
                    double panel_edge_distance = total_thickness;
                    Eigen::Vector3d panel_to_point_direction(0, 0, 0);
                    for (int l = 0; l < (int)d->surface->panelNodes[i].size(); l++) {
                        int next_l;
                        if (l == (int)d->surface->panelNodes[i].size() - 1)
                            next_l = 0;
                        else
                            next_l = l + 1;

                        Eigen::Vector3d point_a = d->surface->transformedPanelPoints[i][l];
                        Eigen::Vector3d point_b = d->surface->transformedPanelPoints[i][next_l];

                        Eigen::Vector3d edge = point_b - point_a;
                        Eigen::Vector3d normal(-edge(1), edge(0), 0.0);
                        normal.normalize();

                        // We are above the panel if the projection lies inside all four panel edges:
                        double normal_component = (x_transformed - point_a).dot(normal);
                        if (normal_component <= 0)
                            projection_in_panel = false;

                        double edge_distance = sqrt(pow(normal_component, 2) + pow(x_transformed(2), 2));
                        if (edge_distance < panel_edge_distance) {
                            // Does the point lie beside the panel edge?
                            if (edge.dot(x_transformed - point_a) * edge.dot(x_transformed - point_b) < 0) {
                                panel_edge_distance = edge_distance;
                                if (edge_distance > 0)
                                    panel_to_point_direction = (normal * normal_component + Eigen::Vector3d(0, 0, x_transformed(2))).normalized();
                            }

                            // Is the point close to the panel vertex?
                            Eigen::Vector3d delta = x_transformed - point_a;
                            double node_distance = delta.norm();
                            if (node_distance < panel_edge_distance) {
                                panel_edge_distance = node_distance;
                                if (node_distance > 0)
                                    panel_to_point_direction = delta.normalized();
                            }
                        }
                    }

                    // Compute distance to panel:
                    double panel_distance;
                    Eigen::Vector3d to_point_direction;
                    if (projection_in_panel) {
                        panel_distance = normal_distance;
                        to_point_direction = Eigen::Vector3d(0, 0, -1);
                    }
                    else {
                        panel_distance = panel_edge_distance;
                        to_point_direction = panel_to_point_direction;
                    }

                    // Are we close to the panel?
                    if (panel_distance < total_thickness) {
                        // Yes. 
                        Eigen::Vector3d velocity;

                        if (panel_distance < boundary_layer_thickness) {
                            // We are in the boundary layer:
                            velocity = bd->boundaryLayer->velocity(d->surface, i, panel_distance);

                        }
                        else if (panel_distance > 0) {
                            // We are in the interpolation layer.
                            // Interpolate between the surface velocity, and the velocity away from the body:
                            Eigen::Vector3d lower_velocity = surfaceVelocity(d->surface, i);

                            // This point lies in the control volume only, if A) no other body lies in the way, and B) the exterior angles\ are more than 90 degrees each.
                            Eigen::Vector3d upper_point_transformed = x_transformed + (total_thickness - panel_distance) * to_point_direction;
                            Eigen::Vector3d upper_point = d->surface->panelCoordinateTransformation(i).inverse() * upper_point_transformed;

                            Eigen::Vector3d upper_velocity;
                            if (in_exterior) {
                                // Compute the upper velocity again using interpolation, in case we are now close to another panel.  This can happen in concave corners.
                                // We must take care, however, to avoid the possibility of an infinite loop.
                                std::set<int> ignore_set_copy(ignoreSet);
                                ignore_set_copy.insert(i);
                                upper_velocity = computeVelocityInterpolated(upper_point, ignore_set_copy);

                            }
                            else {
                                // In the interior, we have the undisturbed freestream velocity.
                                upper_velocity = freestream_velocity;

                            }

                            // Interpolate:
                            double interpolation_distance = panel_distance - boundary_layer_thickness;
                            velocity = (interpolation_distance * upper_velocity + (interpolation_layer_thickness - interpolation_distance) * lower_velocity)
                                / interpolation_layer_thickness;
                        }
                        else {
                            // We are on the panel.  Use surface velocity:
                            velocity = surfaceVelocity(d->surface, i);

                        }

                        // Store interpolated velocity.  We cannot return here.  In concave corners, a point may be close to more than one panel.
                        if (in_exterior) {
                            if (projection_in_panel)
                                close_panel_velocities.push_back(velocity);
                            else
                                close_panel_edge_velocities.push_back(velocity);
                        }
                        else {
                            if (projection_in_panel)
                                interior_close_panel_velocities.push_back(velocity);
                            else
                                interior_close_panel_edge_velocities.push_back(velocity);
                        }
                    }
                }
            }

            offset += d->surface->getNumberOfPanels();
        }
    }

    // Are we close to any panels?   
    for (int i = 0; i < (int)velocity_lists.size(); i++) {
        // Yes, at primacy level i:
        if (velocity_lists[i]->size() > 0) {
            // Average:
            Eigen::Vector3d velocity = Eigen::Vector3d(0, 0, 0);
            std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> >::iterator it;
            for (it = velocity_lists[i]->begin(); it != velocity_lists[i]->end(); it++)
                velocity += *it;

            // Normalize velocity.  The weights sum up to (n - 1) times the sum of the distances.
            velocity /= velocity_lists[i]->size() * 1.0;

            return velocity;
        }
    }

    // No close panels.  Compute potential velocity:
    return computeVelocity(x);
}

Eigen::Vector3d LiftingSurfaceSolver::computeVelocity(const Eigen::Vector3d& x) const
{
    Eigen::Vector3d velocity = Eigen::Vector3d(0, 0, 0);

    int offset = 0;

    // Iterate bodies:
    std::vector<std::shared_ptr<BladeBodyData> >::const_iterator bdi;
    for (bdi = bodies.begin(); bdi != bodies.end(); bdi++) {
        const std::shared_ptr<BladeBodyData>& bd = *bdi;

        // Iterate surfaces:
        std::vector<std::shared_ptr<SurfaceData> > surfaces;
        std::vector<std::shared_ptr<LiftingSurfaceData> >::const_iterator lsi;
        std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
        for (si = bd->bladeBody->nonLiftingSurfaces.begin(); si != bd->bladeBody->nonLiftingSurfaces.end(); si++)
            surfaces.push_back(*si);
        for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++)
            surfaces.push_back(*lsi);

        for (si = surfaces.begin(); si != surfaces.end(); si++) {
            const std::shared_ptr<SurfaceData>& d = *si;

            for (int i = 0; i < d->surface->getNumberOfPanels(); i++) {
                // If no close panels were detected so far, add the influence of this panel:
                velocity += d->surface->vortexRingUnitVelocity(x, i) * doubletCoefficients(offset + i);
                velocity += d->surface->sourceUnitVelocity(x, i) * sourceCoefficients(offset + i);
            }

            offset += d->surface->getNumberOfPanels();
        }

        // If no close panels were detected so far, add the influence of the wakes:
        for (lsi = bd->bladeBody->liftingSurfaces.begin(); lsi != bd->bladeBody->liftingSurfaces.end(); lsi++) {
            const std::shared_ptr<LiftingSurfaceData>& d = *lsi;

            if (d->wake->getNumberOfPanels() >= d->liftingSurface->getSpanwisePanelsCount()) {
                for (int i = 0; i < d->wake->getNumberOfPanels(); i++)
                    velocity += d->wake->vortexRingUnitVelocity(x, i) * d->wake->doubletCoefficients[i];
            }
        }
    }

    // Done:
    return velocity + freestream_velocity;
}

Eigen::Vector3d LiftingSurfaceSolver::computeTrailingEdgeVortexDisplacement(const std::shared_ptr<BladeBodyManager>& bladeBody, const std::shared_ptr<LiftingSurface>& liftingSurface, int index, double dt) const
{
    Eigen::Vector3d apparent_velocity = bladeBody->nodeKinematicVelocity(liftingSurface, liftingSurface->trailingEdgeNode(index)) - freestream_velocity;

    Eigen::Vector3d wakeEmissionVelocity = liftingSurface->wakeEmissionVelocity(apparent_velocity, index);

    return LiftingSurfaceParameters::wakeEmissionDistanceFactor * wakeEmissionVelocity * dt;
}

int LiftingSurfaceSolver::computeIndex(const std::shared_ptr<Surface>& surface, int panel) const
{
    int offset = 0;
    std::vector<std::shared_ptr<SurfaceData> >::const_iterator si;
    for (si = nonWakeSurfaces.begin(); si != nonWakeSurfaces.end(); si++) {
        const std::shared_ptr<SurfaceData>& d = *si;

        if (surface == d->surface)
            return offset + panel;

        offset += d->surface->getNumberOfPanels();
    }

    return -1;
}
