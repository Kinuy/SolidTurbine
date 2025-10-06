#include "BladeBodyManager.h"

BladeBodyManager::BladeBodyManager(const std::string& id) : id(id)
{
    // Initialize kinematics:
    position = Eigen::Vector3d(0, 0, 0);
    velocity = Eigen::Vector3d(0, 0, 0);

    attitude = Eigen::Quaterniond(1, 0, 0, 0);
    rotationalVelocity = Eigen::Vector3d(0, 0, 0);
}

BladeBodyManager::~BladeBodyManager()
{
}

void BladeBodyManager::addNonLiftingSurface(std::shared_ptr<Surface> nonLiftingSurface)
{
    nonLiftingSurfaces.push_back(std::shared_ptr<SurfaceData>(new SurfaceData(nonLiftingSurface)));
}

void BladeBodyManager::addLiftingSurface(std::shared_ptr<LiftingSurface> liftingSurface)
{
    std::shared_ptr<Wake> wake(new Wake(liftingSurface));

    addLiftingSurface(liftingSurface, wake);
}

void BladeBodyManager::addLiftingSurface(std::shared_ptr<LiftingSurface> liftingSurface, std::shared_ptr<Wake> wake)
{
    liftingSurfaces.push_back(std::shared_ptr<LiftingSurfaceData>(new LiftingSurfaceData(liftingSurface, wake)));
}

void BladeBodyManager::stitchPanels(std::shared_ptr<Surface> surfaceA, int panelA, int edgeA, std::shared_ptr<Surface> surfaceB, int panelB, int edgeB)
{
    // Add stitch from A to B:
    stitches[SurfacePanelEdgeData(surfaceA, panelA, edgeA)].push_back(SurfacePanelEdgeData(surfaceB, panelB, edgeB));

    // Add stitch from B to A:
    stitches[SurfacePanelEdgeData(surfaceB, panelB, edgeB)].push_back(SurfacePanelEdgeData(surfaceA, panelA, edgeA));
}

std::vector<SurfacePanelEdgeData> BladeBodyManager::panelNeighbors(const std::shared_ptr<Surface>& surface, int panel) const
{
    std::vector<SurfacePanelEdgeData> neighbors;

    // List in-surface neighbors:
    for (int i = 0; i < (int)surface->panelNodes[panel].size(); i++) {
        std::vector<std::pair<int, int> >& edge_neighbors = surface->panelNeighbors[panel][i];
        std::vector<std::pair<int, int> >::const_iterator it;
        for (it = edge_neighbors.begin(); it != edge_neighbors.end(); it++)
            neighbors.push_back(SurfacePanelEdgeData(surface, it->first, it->second));
    }

    // List stitches:
    for (int i = 0; i < (int)surface->panelNodes[panel].size(); i++) {
        std::map<SurfacePanelEdgeData, std::vector<SurfacePanelEdgeData>, CompareSurfacePanelEdge>::const_iterator it =
            stitches.find(SurfacePanelEdgeData(surface, panel, i));
        if (it != stitches.end()) {
            std::vector<SurfacePanelEdgeData> edge_stitches = it->second;
            std::vector<SurfacePanelEdgeData>::const_iterator sit;
            for (sit = edge_stitches.begin(); sit != edge_stitches.end(); sit++)
                neighbors.push_back(*sit);
        }
    }

    // Done:
    return neighbors;
}

std::vector<SurfacePanelEdgeData> BladeBodyManager::panelNeighbors(const std::shared_ptr<Surface>& surface, int panel, int edge) const
{
    std::vector<SurfacePanelEdgeData> neighbors;

    // List in-surface neighbors:
    {
        std::vector<std::pair<int, int> >& edge_neighbors = surface->panelNeighbors[panel][edge];
        std::vector<std::pair<int, int> >::const_iterator it;
        for (it = edge_neighbors.begin(); it != edge_neighbors.end(); it++)
            neighbors.push_back(SurfacePanelEdgeData(surface, it->first, it->second));
    }

    // List stitches:
    {
        std::map<SurfacePanelEdgeData, std::vector<SurfacePanelEdgeData>, CompareSurfacePanelEdge>::const_iterator it =
            stitches.find(SurfacePanelEdgeData(surface, panel, edge));
        if (it != stitches.end()) {
            std::vector<SurfacePanelEdgeData> edge_stitches = it->second;
            std::vector<SurfacePanelEdgeData>::const_iterator sit;
            for (sit = edge_stitches.begin(); sit != edge_stitches.end(); sit++)
                neighbors.push_back(*sit);
        }
    }

    // Done:
    return neighbors;
}

void BladeBodyManager::setPosition(const Eigen::Vector3d& position)
{
    // Compute differential translation:
    Eigen::Vector3d translation = position - this->position;

    // Apply:
    std::vector<std::shared_ptr<SurfaceData> >::iterator si;
    for (si = nonLiftingSurfaces.begin(); si != nonLiftingSurfaces.end(); si++) {
        std::shared_ptr<SurfaceData> d = *si;

        d->surface->translate(translation);
    }

    std::vector<std::shared_ptr<LiftingSurfaceData> >::iterator lsi;
    for (lsi = liftingSurfaces.begin(); lsi != liftingSurfaces.end(); lsi++) {
        std::shared_ptr<LiftingSurfaceData> d = *lsi;

        d->surface->translate(translation);

        d->wake->translateTrailingEdge(translation);
    }

    // Update state:
    this->position = position;
}

void BladeBodyManager::setAttitude(const Eigen::Quaterniond& attitude)
{
    // Compute differential transformation:
    Eigen::Transform<double, 3, Eigen::Affine> transformation = Eigen::Translation<double, 3>(position) * attitude * this->attitude.inverse() * Eigen::Translation<double, 3>(-position);

    // Apply:
    std::vector<std::shared_ptr<SurfaceData> >::iterator si;
    for (si = nonLiftingSurfaces.begin(); si != nonLiftingSurfaces.end(); si++) {
        std::shared_ptr<SurfaceData> d = *si;

        d->surface->transform(transformation);
    }

    std::vector<std::shared_ptr<LiftingSurfaceData> >::iterator lsi;
    for (lsi = liftingSurfaces.begin(); lsi != liftingSurfaces.end(); lsi++) {
        std::shared_ptr<LiftingSurfaceData> d = *lsi;

        d->surface->transform(transformation);

        d->wake->transformTrailingEdge(transformation);
    }

    // Update state:
    this->attitude = attitude;
}

void BladeBodyManager::setVelocity(const Eigen::Vector3d& velocity)
{
    this->velocity = velocity;
}

void BladeBodyManager::setRotationalVelocity(const Eigen::Vector3d& rotational_velocity)
{
    this->rotationalVelocity = rotational_velocity;
}

Eigen::Vector3d BladeBodyManager::panelKinematicVelocity(const std::shared_ptr<Surface>& surface, int panel) const
{
    const Eigen::Vector3d& panel_position = surface->panelCollocationPoint(panel, false);
    Eigen::Vector3d r = panel_position - position;
    return velocity + rotationalVelocity.cross(r);
}

Eigen::Vector3d BladeBodyManager::nodeKinematicVelocity(const std::shared_ptr<Surface>& surface, int node) const
{
    Eigen::Vector3d r = surface->nodes[node] - position;
    return velocity + rotationalVelocity.cross(r);
}
