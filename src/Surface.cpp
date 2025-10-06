
#include "Surface.h"

Surface::Surface(const std::string& id) : id(id)
{
}

Surface::~Surface()
{
}

int Surface::addTriangle(int node_a, int node_b, int node_c)
{
    std::vector<int> single_panel_nodes;
    single_panel_nodes.push_back(node_a);
    single_panel_nodes.push_back(node_b);
    single_panel_nodes.push_back(node_c);

    int panel_id = (int)panelNodes.size();
    panelNodes.push_back(single_panel_nodes);

    panelNodeNeighbors[node_a]->push_back(panel_id);
    panelNodeNeighbors[node_b]->push_back(panel_id);
    panelNodeNeighbors[node_c]->push_back(panel_id);

    return panel_id;
}

int Surface::addQuadrangle(int node_a, int node_b, int node_c, int node_d)
{
    std::vector<int> single_panel_nodes;
    single_panel_nodes.push_back(node_a);
    single_panel_nodes.push_back(node_b);
    single_panel_nodes.push_back(node_c);
    single_panel_nodes.push_back(node_d);

    int panel_id = (int)panelNodes.size();
    panelNodes.push_back(single_panel_nodes);

    panelNodeNeighbors[node_a]->push_back(panel_id);
    panelNodeNeighbors[node_b]->push_back(panel_id);
    panelNodeNeighbors[node_c]->push_back(panel_id);
    panelNodeNeighbors[node_d]->push_back(panel_id);

    return panel_id;
}

void Surface::calcTopology()
{
    // Compute panel neighbors:
    for (int i = 0; i < (int)panelNodes.size(); i++) {
        std::vector<std::vector<std::pair<int, int> > > single_panel_neighbors;
        single_panel_neighbors.resize(panelNodes[i].size());

        // Every node gives rise to one edge, which gives rise to at most one neighbor.
        for (int j = 0; j < (int)panelNodes[i].size(); j++) {
            // Compute index for next node.
            int next_j;
            if (j == (int)panelNodes[i].size() - 1)
                next_j = 0;
            else
                next_j = j + 1;

            int node = panelNodes[i][j];
            int next_node = panelNodes[i][next_j];

            for (int k = 0; k < (int)panelNodeNeighbors[node]->size(); k++) {
                int potential_neighbor = (*panelNodeNeighbors[node])[k];
                if (potential_neighbor == i)
                    continue;

                // Is this neighbor shared with the next node?   
                if (find(panelNodeNeighbors[next_node]->begin(),
                    panelNodeNeighbors[next_node]->end(),
                    potential_neighbor) != panelNodeNeighbors[next_node]->end()) {
                    // Yes: Establish neighbor relationship.
                    int potential_neighbor_edge = -1;
                    for (int l = 0; l < (int)panelNodes[potential_neighbor].size(); l++) {
                        int next_l;
                        if (l == (int)panelNodes[potential_neighbor].size() - 1)
                            next_l = 0;
                        else
                            next_l = l + 1;

                        int potential_neighbor_node = panelNodes[potential_neighbor][l];
                        int potential_neighbor_next_node = panelNodes[potential_neighbor][next_l];

                        if (find(panelNodeNeighbors[potential_neighbor_node]->begin(),
                            panelNodeNeighbors[potential_neighbor_node]->end(),
                            i) != panelNodeNeighbors[potential_neighbor_node]->end() &&
                            find(panelNodeNeighbors[potential_neighbor_next_node]->begin(),
                                panelNodeNeighbors[potential_neighbor_next_node]->end(),
                                i) != panelNodeNeighbors[potential_neighbor_next_node]->end()) {
                            potential_neighbor_edge = l;
                            break;
                        }
                    }

                    assert(potential_neighbor_edge >= 0);

                    single_panel_neighbors[j].push_back(std::make_pair(potential_neighbor, potential_neighbor_edge));
                }
            }
        }

        panelNeighbors.push_back(single_panel_neighbors);
    }
}

void Surface::cutPanels(int panel_a, int panel_b)
{
    for (int i = 0; i < 2; i++) {
        int panel_ids[2];
        if (i == 0) {
            panel_ids[0] = panel_a;
            panel_ids[1] = panel_b;
        }
        else {
            panel_ids[0] = panel_b;
            panel_ids[1] = panel_a;
        }

        std::vector<std::vector<std::pair<int, int> > >& single_panel_neighbors = panelNeighbors[panel_ids[0]];
        std::vector<std::vector<std::pair<int, int> > >::iterator it;
        for (it = single_panel_neighbors.begin(); it != single_panel_neighbors.end(); it++) {
            std::vector<std::pair<int, int> >& edge_neighbors = *it;
            std::vector<std::pair<int, int> >::iterator sit;
            for (sit = edge_neighbors.begin(); sit != edge_neighbors.end(); ) {
                if (sit->first == panel_ids[1])
                    sit = edge_neighbors.erase(sit);
                else
                    sit++;
            }
        }
    }
}

void Surface::calcGeometry(int panel)
{
    // Resize arrays, if necessary:
    panelNormals.resize(getNumberOfPanels());
    panelCollocationPoints[0].resize(getNumberOfPanels());
    panelCollocationPoints[1].resize(getNumberOfPanels());
    panelCoordinateTransformations.resize(getNumberOfPanels());
    transformedPanelPoints.resize(getNumberOfPanels());
    panelSurfaceAreas.resize(getNumberOfPanels());

    // Get panel nodes:
    std::vector<int>& single_panel_nodes = panelNodes[panel];

    // Normal:
    Eigen::Vector3d normal;
    if (single_panel_nodes.size() == 3) {
        Eigen::Vector3d AB = nodes[single_panel_nodes[1]] - nodes[single_panel_nodes[0]];
        Eigen::Vector3d AC = nodes[single_panel_nodes[2]] - nodes[single_panel_nodes[0]];

        normal = AB.cross(AC);

    }
    else { // 4 sides
        Eigen::Vector3d AC = nodes[single_panel_nodes[2]] - nodes[single_panel_nodes[0]];
        Eigen::Vector3d BD = nodes[single_panel_nodes[3]] - nodes[single_panel_nodes[1]];

        normal = AC.cross(BD);
    }

    normal.normalize();

    panelNormals[panel] = normal;

    // Collocation point:
    Eigen::Vector3d collocation_point(0, 0, 0);
    for (int j = 0; j < (int)single_panel_nodes.size(); j++)
        collocation_point = collocation_point + nodes[single_panel_nodes[j]];

    collocation_point = collocation_point / single_panel_nodes.size();

    panelCollocationPoints[0][panel] = collocation_point;

    Eigen::Vector3d below_surface_collocation_point = collocation_point + LiftingSurfaceParameters::rlCollocationPointDelta * normal;
    panelCollocationPoints[1][panel] = below_surface_collocation_point;

    // Coordinate transformation:
    Eigen::Vector3d AB = nodes[single_panel_nodes[1]] - nodes[single_panel_nodes[0]];
    AB.normalize();

    Eigen::Matrix3d rotation;
    rotation.row(0) = AB;
    rotation.row(1) = normal.cross(AB).normalized(); // Should be normalized already.
    rotation.row(2) = normal;

    Eigen::Transform<double, 3, Eigen::Affine> transformation = rotation * Eigen::Translation<double, 3>(-collocation_point);

    panelCoordinateTransformations[panel] = transformation;

    // Create transformed points.
    std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > single_panel_transformed_points;
    single_panel_transformed_points.reserve(single_panel_nodes.size());
    for (int j = 0; j < (int)single_panel_nodes.size(); j++)
        single_panel_transformed_points.push_back(transformation * nodes[single_panel_nodes[j]]);

    transformedPanelPoints[panel] = single_panel_transformed_points;

    // Surface area: 
    double surface_area = 0.0;
    if (single_panel_nodes.size() == 3) {
        Eigen::Vector3d AB = nodes[single_panel_nodes[1]] - nodes[single_panel_nodes[0]];
        Eigen::Vector3d AC = nodes[single_panel_nodes[2]] - nodes[single_panel_nodes[0]];

        surface_area = 0.5 * AB.cross(AC).norm();

    }
    else { // 4 sides
        Eigen::Vector3d AC = nodes[single_panel_nodes[2]] - nodes[single_panel_nodes[0]];
        Eigen::Vector3d BD = nodes[single_panel_nodes[3]] - nodes[single_panel_nodes[1]];

        surface_area = 0.5 * AC.cross(BD).norm();
    }

    panelSurfaceAreas[panel] = surface_area;
}

void Surface::calcGeometry()
{
    std::cout << "compute surface geometry -> ";

    for (int i = 0; i < getNumberOfPanels(); i++)
        calcGeometry(i);
}

int Surface::getNumberOfNodes() const
{
    return (int)nodes.size();
}

int Surface::getNumberOfPanels() const
{
    return (int)panelNodes.size();
}

void Surface::rotate(const Eigen::Vector3d& axis, double angle)
{

    transform(Eigen::AngleAxis<double>(angle, axis).toRotationMatrix());
}

void Surface::transform(const Eigen::Matrix3d& transformation)
{
    transform(Eigen::Transform<double, 3, Eigen::Affine>(transformation));
}

void Surface::transform(const Eigen::Transform<double, 3, Eigen::Affine>& transformation)
{
    for (int i = 0; i < getNumberOfNodes(); i++)
        nodes[i] = transformation * nodes[i];

    for (int j = 0; j < 2; j++)
        for (int i = 0; i < getNumberOfPanels(); i++)
            panelCollocationPoints[j][i] = transformation * panelCollocationPoints[j][i];

    for (int i = 0; i < getNumberOfPanels(); i++)
        panelNormals[i] = transformation.linear() * panelNormals[i];

    for (int i = 0; i < getNumberOfPanels(); i++)
        panelCoordinateTransformations[i] = panelCoordinateTransformations[i] * transformation.inverse();
}

void Surface::translate(const Eigen::Vector3d& translation)
{
    for (int i = 0; i < getNumberOfNodes(); i++)
        nodes[i] = nodes[i] + translation;

    for (int j = 0; j < 2; j++)
        for (int i = 0; i < getNumberOfPanels(); i++)
            panelCollocationPoints[j][i] = panelCollocationPoints[j][i] + translation;

    for (int i = 0; i < getNumberOfPanels(); i++)
        panelCoordinateTransformations[i] = panelCoordinateTransformations[i] * Eigen::Translation<double, 3>(-translation);
}

const Eigen::Vector3d& Surface::panelCollocationPoint(int panel, bool below_surface) const
{
    return panelCollocationPoints[below_surface][panel];
}

const Eigen::Vector3d& Surface::panelNormal(int panel) const
{
    return panelNormals[panel];
}

const Eigen::Transform<double, 3, Eigen::Affine>& Surface::panelCoordinateTransformation(int panel) const
{
    return panelCoordinateTransformations[panel];
}

double Surface::panelSurfaceArea(int panel) const
{
    return panelSurfaceAreas[panel];
}

static void source_and_doublet_edge_influence(const Eigen::Vector3d& x, const Eigen::Vector3d& node_a, const Eigen::Vector3d& node_b, double* source_edge_influence, double* doublet_edge_influence)
{
    double d = sqrt(pow(node_b(0) - node_a(0), 2) + pow(node_b(1) - node_a(1), 2));

    if (d < LiftingSurfaceParameters::zeroThreshold) {
        if (source_edge_influence != NULL)
            *source_edge_influence = 0.0;
        if (doublet_edge_influence != NULL)
            *doublet_edge_influence = 0.0;

        return;
    }

    double z = x(2);

    double m = (node_b(1) - node_a(1)) / (node_b(0) - node_a(0));

    double e1 = pow(x(0) - node_a(0), 2) + pow(z, 2);
    double e2 = pow(x(0) - node_b(0), 2) + pow(z, 2);

    double r1 = sqrt(e1 + pow(x(1) - node_a(1), 2));
    double r2 = sqrt(e2 + pow(x(1) - node_b(1), 2));

    double h1 = (x(0) - node_a(0)) * (x(1) - node_a(1));
    double h2 = (x(0) - node_b(0)) * (x(1) - node_b(1));

    // IEEE-754 floating point division by zero results in +/- inf, and atan(inf) = pi / 2.
    double u = (m * e1 - h1) / (z * r1);
    double v = (m * e2 - h2) / (z * r2);

    double delta_theta;
    if (u == v)
        delta_theta = 0.0;
    else
        delta_theta = atan2(u - v, 1 + u * v);

    if (source_edge_influence != NULL)
        *source_edge_influence = ((x(0) - node_a(0)) * (node_b(1) - node_a(1)) - (x(1) - node_a(1)) * (node_b(0) - node_a(0))) / d * log((r1 + r2 + d) / (r1 + r2 - d)) - fabs(z) * delta_theta;
    if (doublet_edge_influence != NULL)
        *doublet_edge_influence = delta_theta;
}

void Surface::sourceAndDoubletInfluence(const Eigen::Vector3d& x, int this_panel, double& sourceInfluence, double& doubletInfluence) const
{
    // Transform such that panel normal becomes unit Z vector:    
    const Eigen::Transform<double, 3, Eigen::Affine>& transformation = panelCoordinateTransformation(this_panel);

    Eigen::Vector3d x_normalized = transformation * x;

    // Compute influence coefficient according to Hess:
    sourceInfluence = 0.0;
    doubletInfluence = 0.0;

    for (int i = 0; i < (int)panelNodes[this_panel].size(); i++) {
        int next_idx;
        if (i == (int)panelNodes[this_panel].size() - 1)
            next_idx = 0;
        else
            next_idx = i + 1;

        const Eigen::Vector3d& node_a = transformedPanelPoints[this_panel][i];
        const Eigen::Vector3d& node_b = transformedPanelPoints[this_panel][next_idx];

        double source_edge_influence, doublet_edge_influence;

        source_and_doublet_edge_influence(x_normalized, node_a, node_b, &source_edge_influence, &doublet_edge_influence);

        sourceInfluence += source_edge_influence;
        doubletInfluence += doublet_edge_influence;
    }

    sourceInfluence *= -MathUtility::getOneOver4Pi();
    doubletInfluence *= MathUtility::getOneOver4Pi();
}

double Surface::sourceInfluence(const Eigen::Vector3d& x, int this_panel) const
{
    // Transform such that panel normal becomes unit Z vector:    
    const Eigen::Transform<double, 3, Eigen::Affine>& transformation = panelCoordinateTransformation(this_panel);

    Eigen::Vector3d x_normalized = transformation * x;

    // Compute influence coefficient according to Hess:
    double influence = 0.0;
    for (int i = 0; i < (int)panelNodes[this_panel].size(); i++) {
        int next_idx;
        if (i == (int)panelNodes[this_panel].size() - 1)
            next_idx = 0;
        else
            next_idx = i + 1;

        const Eigen::Vector3d& node_a = transformedPanelPoints[this_panel][i];
        const Eigen::Vector3d& node_b = transformedPanelPoints[this_panel][next_idx];

        double edge_influence;
        source_and_doublet_edge_influence(x_normalized, node_a, node_b, &edge_influence, NULL);

        influence += edge_influence;
    }

    return -MathUtility::getOneOver4Pi() * influence;
}

double Surface::doubletInfluence(const Eigen::Vector3d& x, int this_panel) const
{
    // Transform such that panel normal becomes unit Z vector:
    const Eigen::Transform<double, 3, Eigen::Affine>& transformation = panelCoordinateTransformation(this_panel);

    Eigen::Vector3d x_normalized = transformation * x;

    // Compute influence coefficient according to Hess:
    double influence = 0.0;
    for (int i = 0; i < (int)panelNodes[this_panel].size(); i++) {
        int next_idx;
        if (i == (int)panelNodes[this_panel].size() - 1)
            next_idx = 0;
        else
            next_idx = i + 1;

        const Eigen::Vector3d& node_a = transformedPanelPoints[this_panel][i];
        const Eigen::Vector3d& node_b = transformedPanelPoints[this_panel][next_idx];

        double edge_influence;
        source_and_doublet_edge_influence(x_normalized, node_a, node_b, NULL, &edge_influence);

        influence += edge_influence;
    }

    return  MathUtility::getOneOver4Pi() * influence;
}

static Eigen::Vector3d source_edge_unit_velocity(const Eigen::Vector3d& x, const Eigen::Vector3d& node_a, const Eigen::Vector3d& node_b)
{
    double d = sqrt(pow(node_b(0) - node_a(0), 2) + pow(node_b(1) - node_a(1), 2));

    if (d < LiftingSurfaceParameters::zeroThreshold)
        return Eigen::Vector3d(0, 0, 0);

    double z = x(2);

    double m = (node_b(1) - node_a(1)) / (node_b(0) - node_a(0));

    double e1 = pow(x(0) - node_a(0), 2) + pow(z, 2);
    double e2 = pow(x(0) - node_b(0), 2) + pow(z, 2);

    double r1 = sqrt(e1 + pow(x(1) - node_a(1), 2));
    double r2 = sqrt(e2 + pow(x(1) - node_b(1), 2));

    double h1 = (x(0) - node_a(0)) * (x(1) - node_a(1));
    double h2 = (x(0) - node_b(0)) * (x(1) - node_b(1));

    // IEEE-754 floating point division by zero results in +/- inf, and atan(inf) = pi / 2.
    double u = (m * e1 - h1) / (z * r1);
    double v = (m * e2 - h2) / (z * r2);

    double delta_theta;
    if (u == v)
        delta_theta = 0.0;
    else
        delta_theta = atan2(u - v, 1 + u * v);

    double l = log((r1 + r2 - d) / (r1 + r2 + d));

    return Eigen::Vector3d((node_b(1) - node_a(1)) / d * l,
        (node_a(0) - node_b(0)) / d * l,
        delta_theta);
}

Eigen::Vector3d Surface::sourceUnitVelocity(const Eigen::Vector3d& x, int this_panel) const
{
    // Transform such that panel normal becomes unit Z vector:
    const Eigen::Transform<double, 3, Eigen::Affine>& transformation = panelCoordinateTransformation(this_panel);

    Eigen::Vector3d x_normalized = transformation * x;

    // Compute influence coefficient according to Hess:
    Eigen::Vector3d velocity(0, 0, 0);
    for (int i = 0; i < (int)panelNodes[this_panel].size(); i++) {
        int next_idx;
        if (i == (int)panelNodes[this_panel].size() - 1)
            next_idx = 0;
        else
            next_idx = i + 1;

        const Eigen::Vector3d& node_a = transformedPanelPoints[this_panel][i];
        const Eigen::Vector3d& node_b = transformedPanelPoints[this_panel][next_idx];

        velocity += source_edge_unit_velocity(x_normalized, node_a, node_b);
    }

    // Transform back:
    velocity = transformation.linear().transpose() * velocity;

    // Done:
    return  MathUtility::getOneOver4Pi() * velocity;
}

Eigen::Vector3d Surface::vortexRingUnitVelocity(const Eigen::Vector3d& x, int this_panel) const
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

        double r_1_norm = r_1.norm();
        double r_2_norm = r_2.norm();

        Eigen::Vector3d r_1xr_2 = r_1.cross(r_2);
        double r_1xr_2_sqnorm = r_1xr_2.squaredNorm();

        if (r_1_norm < LiftingSurfaceParameters::zeroThreshold ||
            r_2_norm < LiftingSurfaceParameters::zeroThreshold ||
            r_1xr_2_sqnorm < LiftingSurfaceParameters::zeroThreshold)
            continue;

        velocity += r_1xr_2 / r_1xr_2_sqnorm * r_0.dot(r_1 / r_1_norm - r_2 / r_2_norm);
    }

    return  MathUtility::getOneOver4Pi() * velocity;
}

double Surface::doubletInfluence(const std::shared_ptr<Surface>& other, int other_panel, int this_panel) const
{
    if ((this == other.get()) && (this_panel == other_panel))
        return -0.5;
    else
        return doubletInfluence(other->panelCollocationPoint(other_panel, true), this_panel);
}

double Surface::sourceInfluence(const std::shared_ptr<Surface>& other, int other_panel, int this_panel) const
{
    return sourceInfluence(other->panelCollocationPoint(other_panel, true), this_panel);
}

void Surface::sourceAndDoubletInfluence(const std::shared_ptr<Surface>& other, int other_panel, int this_panel, double& sourceInfluence, double& doubletInfluence) const
{
    if ((this == other.get()) && (this_panel == other_panel)) {
        doubletInfluence = -0.5;

        sourceInfluence = this->sourceInfluence(other, other_panel, this_panel);

    }
    else
        sourceAndDoubletInfluence(other->panelCollocationPoint(other_panel, true), this_panel, sourceInfluence, doubletInfluence);
}

Eigen::Vector3d Surface::sourceUnitVelocity(const std::shared_ptr<Surface>& other, int other_panel, int this_panel) const
{
    return sourceUnitVelocity(other->panelCollocationPoint(other_panel, true), this_panel);
}

Eigen::Vector3d Surface::vortexRingUnitVelocity(const std::shared_ptr<Surface>& other, int other_panel, int this_panel) const
{
    return vortexRingUnitVelocity(other->panelCollocationPoint(other_panel, true), this_panel);
}