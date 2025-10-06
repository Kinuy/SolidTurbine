
#include "LiftingSurface.h"

LiftingSurface::LiftingSurface(const std::string& id) : Surface(id)
{
}

LiftingSurface::~LiftingSurface()
{
}

int LiftingSurface::getChordwiseNodesCount() const
{
    return (int)nodesTop.rows();
}

int LiftingSurface::getChordwisePanelsCount() const
{
    return (int)panelsTop.rows();
}

int LiftingSurface::getSpanwiseNodesCount() const
{
    return (int)nodesTop.cols();
}

int LiftingSurface::getSpanwisePanelsCount() const
{
    return (int)panelsTop.cols();
}

int LiftingSurface::trailingEdgeNode(int index) const
{
    return nodesTop(nodesTop.rows() - 1, index);
}

int LiftingSurface::trailingEdgeTopPanel(int index) const
{
    return panelsTop(panelsTop.rows() - 1, index);
}

int LiftingSurface::trailingEdgeBottomPanel(int index) const
{
    return panelsBottom(panelsBottom.rows() - 1, index);
}

void LiftingSurface::finishTrailingEdge()
{
    // Compute trailing edge bisectors and normals to the initial wake strip surface:
    trailingEdgeBisectors.resize(getSpanwiseNodesCount(), 3);
    wakeNormals.resize(getSpanwiseNodesCount(), 3);

    if (getChordwiseNodesCount() > 1) {
        for (int i = 0; i < getSpanwiseNodesCount(); i++) {
            // Compute bisector: 
            Eigen::Vector3d upper = nodes[nodesTop(nodesTop.rows() - 1, i)] - nodes[nodesTop(nodesTop.rows() - 2, i)];
            Eigen::Vector3d lower = nodes[nodesBottom(nodesBottom.rows() - 1, i)] - nodes[nodesBottom(nodesBottom.rows() - 2, i)];

            upper.normalize();
            lower.normalize();

            Eigen::Vector3d trailing_edge_bisector = upper + lower;
            trailing_edge_bisector.normalize();

            trailingEdgeBisectors.row(i) = trailing_edge_bisector;

            // Compute normal to the initial wake strip surface, spanned by the bisector and by the span direction:
            int prev_node, next_node;

            if (i > 0)
                prev_node = trailingEdgeNode(i - 1);
            else
                prev_node = trailingEdgeNode(i);

            if (i < getSpanwiseNodesCount() - 1)
                next_node = trailingEdgeNode(i + 1);
            else
                next_node = trailingEdgeNode(i);

            Eigen::Vector3d wake_normal(0, 0, 0);

            if (prev_node != next_node) {
                Eigen::Vector3d span_direction = nodes[next_node] - nodes[prev_node];

                wake_normal = span_direction.cross(trailing_edge_bisector);
                wake_normal.normalize();
            }

            wakeNormals.row(i) = wake_normal;
        }

    }
    else {
        // No bisector information available:
        trailingEdgeBisectors.setZero();
        wakeNormals.setZero();
    }

    // Terminate neighbor relationships across trailing edge.
    for (int i = 0; i < getSpanwisePanelsCount(); i++)
        cutPanels(trailingEdgeTopPanel(i), trailingEdgeBottomPanel(i));
}

void LiftingSurface::transform(const Eigen::Transform<double, 3, Eigen::Affine>& transformation)
{
    this->Surface::transform(transformation);

    // Transform bisectors and wake normals:
    for (int i = 0; i < getSpanwiseNodesCount(); i++) {
        Eigen::Vector3d trailing_edge_bisector = trailingEdgeBisectors.row(i);
        trailingEdgeBisectors.row(i) = transformation.linear() * trailing_edge_bisector;

        Eigen::Vector3d wake_normal = wakeNormals.row(i);
        wakeNormals.row(i) = transformation.linear() * wake_normal;
    }
}

Eigen::Vector3d LiftingSurface::wakeEmissionVelocity(const Eigen::Vector3d& apparentVelocity, int nodeIndex) const
{
    Eigen::Vector3d wakeEmissionVelocity;

    if (LiftingSurfaceParameters::wakeEmissionFollowBisector) {
        Eigen::Vector3d wake_normal = wakeNormals.row(nodeIndex);

        wakeEmissionVelocity = -(apparentVelocity - apparentVelocity.dot(wake_normal) * wake_normal);

    }
    else {
        wakeEmissionVelocity = -apparentVelocity;

    }

    return wakeEmissionVelocity;
}