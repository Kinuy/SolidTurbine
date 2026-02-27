#include "Wake.h"

Wake::Wake(std::shared_ptr<LiftingSurface> lifting_surface)
    : Surface(lifting_surface->id + std::string("_wake")), liftingSurface(lifting_surface)
{
}

void Wake::addLayer()
{
    // Is this the first layer?
    bool first_layer;
    if (getNumberOfNodes() < liftingSurface->getSpanwiseNodesCount())
        first_layer = true;
    else
        first_layer = false;

    // Add layer of nodes at trailing edge, and add panels if necessary:
    for (int k = 0; k < liftingSurface->getSpanwiseNodesCount(); k++) {
        Eigen::Vector3d new_point = liftingSurface->nodes[liftingSurface->trailingEdgeNode(k)];

        int node = getNumberOfNodes();
        nodes.push_back(new_point);

        if (k > 0 && !first_layer) {
            std::vector<int> vertices;
            vertices.push_back(node - 1);
            vertices.push_back(node - 1 - liftingSurface->getSpanwiseNodesCount());
            vertices.push_back(node - liftingSurface->getSpanwiseNodesCount());
            vertices.push_back(node);

            int panel = getNumberOfPanels();
            panelNodes.push_back(vertices);

            std::vector<std::vector<std::pair<int, int> > > local_panel_neighbors;
            local_panel_neighbors.resize(vertices.size());
            panelNeighbors.push_back(local_panel_neighbors);

            std::shared_ptr<std::vector<int> > empty = std::make_shared<std::vector<int> >();
            panelNodeNeighbors.push_back(empty);

            doubletCoefficients.push_back(0);

            calcGeometry(panel);

        }
        else {
            std::shared_ptr<std::vector<int> > empty = std::make_shared<std::vector<int> >();
            panelNodeNeighbors.push_back(empty);
        }
    }
}

void Wake::translateTrailingEdge(const Eigen::Vector3d& translation)
{
    if (getNumberOfNodes() < liftingSurface->getSpanwiseNodesCount())
        return;

    int k0;

    if (LiftingSurfaceParameters::convectWake)
        k0 = getNumberOfNodes() - liftingSurface->getSpanwiseNodesCount();
    else
        k0 = 0;

    for (int k = k0; k < getNumberOfNodes(); k++)
        nodes[k] += translation;

    if (LiftingSurfaceParameters::convectWake)
        k0 = getNumberOfPanels() - liftingSurface->getSpanwisePanelsCount();
    else
        k0 = 0;

    for (int k = k0; k < getNumberOfPanels(); k++)
        calcGeometry(k);
}

void Wake::transformTrailingEdge(const Eigen::Transform<double, 3, Eigen::Affine>& transformation)
{
    if (getNumberOfNodes() < liftingSurface->getSpanwiseNodesCount())
        return;

    int k0;

    if (LiftingSurfaceParameters::convectWake)
        k0 = getNumberOfNodes() - liftingSurface->getSpanwiseNodesCount();
    else
        k0 = 0;

    for (int k = k0; k < getNumberOfNodes(); k++)
        nodes[k] = transformation * nodes[k];

    if (LiftingSurfaceParameters::convectWake)
        k0 = getNumberOfPanels() - liftingSurface->getSpanwisePanelsCount();
    else
        k0 = 0;

    for (int k = k0; k < getNumberOfPanels(); k++)
        calcGeometry(k);
}

void Wake::updateProperties(double)
{
}

Eigen::Vector3d Wake::vortexRingUnitVelocity(const Eigen::Vector3d& x, int this_panel) const
{
    Eigen::Vector3d velocity(0, 0, 0);

    for (int i = 0; i < (int)panelNodes[this_panel].size(); i++) {
        int previous_idx;
        if (i == 0)
            previous_idx = (int)panelNodes[this_panel].size() - 1;
        else
            previous_idx = i - 1;

        const Eigen::Vector3d& node_a = nodes[panelNodes[this_panel][previous_idx]];
        const Eigen::Vector3d& node_b = nodes[panelNodes[this_panel][i]];

        Eigen::Vector3d r_0 = node_b - node_a;
        Eigen::Vector3d r_1 = node_a - x;
        Eigen::Vector3d r_2 = node_b - x;

        double r_0_norm = r_0.norm();
        double r_1_norm = r_1.norm();
        double r_2_norm = r_2.norm();

        Eigen::Vector3d r_1xr_2 = r_1.cross(r_2);
        double r_1xr_2_sqnorm = r_1xr_2.squaredNorm();

        if (r_0_norm < LiftingSurfaceParameters::zeroThreshold ||
            r_1_norm < LiftingSurfaceParameters::zeroThreshold ||
            r_2_norm < LiftingSurfaceParameters::zeroThreshold ||
            r_1xr_2_sqnorm < LiftingSurfaceParameters::zeroThreshold)
            continue;

        double r = sqrt(r_1xr_2_sqnorm) / r_0_norm;
        if (r < LiftingSurfaceParameters::wakeVortexCoreRadius) {
            // Rankine vortex core segment:
            velocity += r_1xr_2 / (r_0_norm * pow(LiftingSurfaceParameters::wakeVortexCoreRadius, 2))
                * (r_0 / r_0_norm).dot(r_1 / r_1_norm - r_2 / r_2_norm);

        }
        else {
            // Free vortex segment:
            velocity += r_1xr_2 / r_1xr_2_sqnorm * r_0.dot(r_1 / r_1_norm - r_2 / r_2_norm);

        }
    }

    return  MathUtility::getOneOver4Pi() * velocity;
}