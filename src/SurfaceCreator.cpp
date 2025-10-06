#include "SurfaceCreator.h"


SurfaceCreator::SurfaceCreator(Surface& surface) : surface(surface)
{
}

std::vector<int> SurfaceCreator::createNodesForPoints(const std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> >& points)
{
    std::vector<int> new_nodes;

    for (int i = 0; i < (int)points.size(); i++) {
        int node_id = (int)surface.nodes.size();

        surface.nodes.push_back(points[i]);

        std::shared_ptr<std::vector<int> > empty_vector = std::make_shared<std::vector<int> >();
        surface.panelNodeNeighbors.push_back(empty_vector);

        new_nodes.push_back(node_id);
    }

    return new_nodes;
}

std::vector<int> SurfaceCreator::createPanelsBetweenShapes(const std::vector<int>& firstNodes, const std::vector<int>& secondNodes, bool cyclic)
{
    std::vector<int> new_panels;

    for (int i = 0; i < (int)firstNodes.size(); i++) {
        // Bundle panel nodes in appropriate order:
        int next_i;
        if (i == (int)firstNodes.size() - 1) {
            if (cyclic)
                next_i = 0;
            else
                break;
        }
        else
            next_i = i + 1;

        std::vector<int> original_nodes;
        original_nodes.push_back(firstNodes[i]);
        original_nodes.push_back(secondNodes[i]);
        original_nodes.push_back(secondNodes[next_i]);
        original_nodes.push_back(firstNodes[next_i]);

        // Filter out duplicate nodes while preserving order:
        std::vector<int> unique_nodes;

        for (int j = 0; j < 4; j++) {
            bool duplicate = false;
            for (int k = j + 1; k < 4; k++) {
                if (original_nodes[j] == original_nodes[k]) {
                    duplicate = true;

                    break;
                }
            }

            if (!duplicate)
                unique_nodes.push_back(original_nodes[j]);
        }

        // Add panel appropriate for number of nodes:
        int panel_id;
        switch (unique_nodes.size()) {
        case 3:
        {
            // Add triangle:
            panel_id = surface.addTriangle(unique_nodes[0], unique_nodes[1], unique_nodes[2]);

            break;
        }

        case 4:
        {
            // Construct a planar quadrangle:

            // Center points around their mean:
            Eigen::Vector3d mean(0, 0, 0);
            for (int i = 0; i < 4; i++)
                mean += surface.nodes[unique_nodes[i]];
            mean /= 4.0;

            Eigen::MatrixXd X(4, 3);
            for (int i = 0; i < 4; i++)
                X.row(i) = surface.nodes[unique_nodes[i]] - mean;

            // Perform PCA to find dominant directions:
            Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(X.transpose() * X);

            Eigen::Vector3d eigenvalues = solver.eigenvalues();
            Eigen::Matrix3d eigenvectors = solver.eigenvectors();

            double min_eigenvalue = std::numeric_limits<double>::max();
            int min_eigenvalue_index = -1;
            for (int i = 0; i < 3; i++) {
                if (eigenvalues(i) < min_eigenvalue) {
                    min_eigenvalue = eigenvalues(i);
                    min_eigenvalue_index = i;
                }
            }

            Eigen::Vector3d normal = eigenvectors.col(min_eigenvalue_index);

            // Create new points by projecting onto surface spanned by dominant directions:
            Eigen::Vector3d vertices[4];
            for (int i = 0; i < 4; i++)
                vertices[i] = surface.nodes[unique_nodes[i]] - (normal * X.row(i)) * normal;

            // Add points to surface:
            int new_nodes[4];
            for (int j = 0; j < 4; j++) {
                // If the new points don't match the original ones, create new nodes:
                if ((vertices[j] - surface.nodes[unique_nodes[j]]).norm() < LiftingSurfaceParameters::zeroThreshold) {
                    new_nodes[j] = unique_nodes[j];
                }
                else {
                    new_nodes[j] = (int)surface.nodes.size();

                    surface.nodes.push_back(vertices[j]);

                    surface.panelNodeNeighbors.push_back(surface.panelNodeNeighbors[unique_nodes[j]]);
                }
            }

            // Add planar quadrangle:
            panel_id = surface.addQuadrangle(new_nodes[0], new_nodes[1], new_nodes[2], new_nodes[3]);

            break;
        }

        default:
            // Unknown panel type:
            std::cerr << "SurfaceCreator::createPanelsBetweenShapes: Cannot create panel with " << unique_nodes.size() << " vertices." << std::endl;
            exit(1);
        }

        new_panels.push_back(panel_id);
    }

    return new_panels;
}

std::vector<int> SurfaceCreator::createPanelsInsideShape(const std::vector<int>& nodes, const Eigen::Vector3d& tipPoint, int zSign)
{
    std::vector<int> new_panels;

    // Add tip node:
    int tip_node = (int)surface.nodes.size();

    surface.nodes.push_back(tipPoint);

    std::shared_ptr<std::vector<int> > empty_vector = std::make_shared<std::vector<int> >();
    surface.panelNodeNeighbors.push_back(empty_vector);

    // Create triangle for leading and trailing edges:
    for (int i = 0; i < (int)nodes.size(); i++) {
        int triangle[3];

        if (zSign == 1) {
            triangle[0] = nodes[i];
            if (i == (int)nodes.size() - 1)
                triangle[1] = nodes[0];
            else
                triangle[1] = nodes[i + 1];
            triangle[2] = tip_node;

        }
        else {
            triangle[0] = nodes[i];
            triangle[1] = tip_node;
            if (i == (int)nodes.size() - 1)
                triangle[2] = nodes[0];
            else
                triangle[2] = nodes[i + 1];

        }

        int new_panel = surface.addTriangle(triangle[0], triangle[1], triangle[2]);
        new_panels.push_back(new_panel);
    }

    // Done:
    return new_panels;
}

void SurfaceCreator::finish()
{
    surface.calcTopology();
    surface.calcGeometry();
}