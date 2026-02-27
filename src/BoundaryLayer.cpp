#include "BoundaryLayer.h"

bool BoundaryLayer::recalculate(const Eigen::Vector3d & /*freestreamVelocity*/, const Eigen::MatrixXd & /*surfaceVelocities*/)
{
    return true;
}

double BoundaryLayer::thickness(const std::shared_ptr<Surface> & /*surface*/, int /*panel*/) const
{
    return 0.0;
}

Eigen::Vector3d BoundaryLayer::velocity(const std::shared_ptr<Surface> & /*surface*/, int /*panel*/, double /*y*/) const
{
    return Eigen::Vector3d(0, 0, 0);
}

double BoundaryLayer::transpirationVelocity(const std::shared_ptr<Surface> & /*surface*/, int /*panel*/) const
{
    return 0.0;
}

Eigen::Vector3d BoundaryLayer::friction(const std::shared_ptr<Surface> & /*surface*/, int /*panel*/) const
{
    return Eigen::Vector3d(0, 0, 0);
}
