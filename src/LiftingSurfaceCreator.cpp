#include "LiftingSurfaceCreator.h"

LiftingSurfaceCreator::LiftingSurfaceCreator(LiftingSurface& liftingSurface) : SurfaceCreator(liftingSurface), liftingSurface(liftingSurface)
{
}

std::vector<int> LiftingSurfaceCreator::createPanelsInsideAirfoil(const std::vector<int>& airfoilNodes, int trailingEdgePointId, int zSign)
{
    std::vector<int> new_panels;

    // Add middle nodes:
    std::vector<int> upper_nodes;
    std::vector<int> lower_nodes;
    std::vector<int> middle_nodes;
    for (int i = 1; i < trailingEdgePointId; i++) {
        int upper_node_id = airfoilNodes[i];
        int lower_node_id = airfoilNodes[airfoilNodes.size() - i];

        Eigen::Vector3d upper_point = liftingSurface.nodes[upper_node_id];
        Eigen::Vector3d lower_point = liftingSurface.nodes[lower_node_id];

        Eigen::Vector3d middle_point = 0.5 * (upper_point + lower_point);

        int middle_node_id = (int)liftingSurface.nodes.size();

        liftingSurface.nodes.push_back(middle_point);

        std::shared_ptr<std::vector<int> > empty_vector = std::make_shared<std::vector<int> >();
        liftingSurface.panelNodeNeighbors.push_back(empty_vector);

        upper_nodes.push_back(upper_node_id);
        lower_nodes.push_back(lower_node_id);
        middle_nodes.push_back(middle_node_id);
    }

    // Close middle part with panels:
    std::vector<int> new_between_panels;

    if (zSign == 1) {
        new_between_panels = createPanelsBetweenShapes(middle_nodes, upper_nodes, false);
        new_panels.insert(new_panels.end(), new_between_panels.begin(), new_between_panels.end());

        new_between_panels = createPanelsBetweenShapes(lower_nodes, middle_nodes, false);
        new_panels.insert(new_panels.end(), new_between_panels.begin(), new_between_panels.end());

    }
    else {
        new_between_panels = createPanelsBetweenShapes(upper_nodes, middle_nodes, false);
        new_panels.insert(new_panels.end(), new_between_panels.begin(), new_between_panels.end());

        new_between_panels = createPanelsBetweenShapes(middle_nodes, lower_nodes, false);
        new_panels.insert(new_panels.end(), new_between_panels.begin(), new_between_panels.end());

    }

    // Create triangle for leading and trailing edges:
    int new_panel;

    if (zSign == 1) {
        new_panel = liftingSurface.addTriangle(airfoilNodes[0], airfoilNodes[1], middle_nodes[0]);
        new_panels.push_back(new_panel);

        new_panel = liftingSurface.addTriangle(airfoilNodes[0], middle_nodes[0], airfoilNodes[airfoilNodes.size() - 1]);
        new_panels.push_back(new_panel);

        new_panel = liftingSurface.addTriangle(middle_nodes[trailingEdgePointId - 2], airfoilNodes[trailingEdgePointId - 1], airfoilNodes[trailingEdgePointId]);
        new_panels.push_back(new_panel);

        new_panel = liftingSurface.addTriangle(middle_nodes[trailingEdgePointId - 2], airfoilNodes[trailingEdgePointId], airfoilNodes[trailingEdgePointId + 1]);
        new_panels.push_back(new_panel);

    }
    else {
        new_panel = liftingSurface.addTriangle(airfoilNodes[0], middle_nodes[0], airfoilNodes[1]);
        new_panels.push_back(new_panel);

        new_panel = liftingSurface.addTriangle(airfoilNodes[0], airfoilNodes[airfoilNodes.size() - 1], middle_nodes[0]);
        new_panels.push_back(new_panel);

        new_panel = liftingSurface.addTriangle(middle_nodes[trailingEdgePointId - 2], airfoilNodes[trailingEdgePointId], airfoilNodes[trailingEdgePointId - 1]);
        new_panels.push_back(new_panel);

        new_panel = liftingSurface.addTriangle(middle_nodes[trailingEdgePointId - 2], airfoilNodes[trailingEdgePointId + 1], airfoilNodes[trailingEdgePointId]);
        new_panels.push_back(new_panel);

    }

    // Done:
    return new_panels;
}

void LiftingSurfaceCreator::finish(const std::vector<std::vector<int> >& node_strips, const std::vector<std::vector<int> >& panel_strips, int trailing_edge_point_id)
{
    // Perform basic surface finishing.
    this->SurfaceCreator::finish();

    // Put together the node matrices.
    // We need to apply some trickery to make sure that the leading and trailing edge nodes appear in both matrices.
    liftingSurface.nodesTop.resize(trailing_edge_point_id + 1, node_strips.size());
    liftingSurface.nodesBottom.resize(node_strips[0].size() - trailing_edge_point_id + 1, node_strips.size());

    for (int i = 0; i <= trailing_edge_point_id; i++) {
        for (int j = 0; j < (int)node_strips.size(); j++) {
            liftingSurface.nodesTop(i, j) = node_strips[j][i];

            if (i == 0)
                liftingSurface.nodesBottom(i, j) = node_strips[j][i];
        }
    }

    for (int i = trailing_edge_point_id; i < (int)node_strips[0].size(); i++)
        for (int j = 0; j < (int)node_strips.size(); j++)
            liftingSurface.nodesBottom(node_strips[0].size() - i, j) = node_strips[j][i];

    // Put together the panel matrices.
    liftingSurface.panelsTop.resize(liftingSurface.nodesTop.rows() - 1, liftingSurface.nodesTop.cols() - 1);
    liftingSurface.panelsBottom.resize(liftingSurface.nodesBottom.rows() - 1, liftingSurface.nodesBottom.cols() - 1);

    for (int i = 0; i < (int)liftingSurface.panelsTop.rows(); i++)
        for (int j = 0; j < (int)liftingSurface.panelsTop.cols(); j++)
            liftingSurface.panelsTop(i, j) = panel_strips[j][i];

    for (int i = 0; i < (int)liftingSurface.panelsBottom.rows(); i++)
        for (int j = 0; j < (int)liftingSurface.panelsBottom.cols(); j++)
            liftingSurface.panelsBottom(liftingSurface.panelsBottom.rows() - 1 - i, j) = panel_strips[j][liftingSurface.panelsTop.rows() + i];

    // Finish trailing edge setup.
    liftingSurface.finishTrailingEdge();
}