
#include "SurfaceWriter.h"

bool SurfaceWriter::write(const std::shared_ptr<Surface>& surface, const std::string& filename)
{
    return write(surface, filename, 0, 0);
}

bool SurfaceWriter::write(const std::shared_ptr<Surface>& surface, const std::string& filename, int node_offset, int panel_offset)
{
    std::vector<std::string> empty_names;
    std::vector<Eigen::MatrixXd, Eigen::aligned_allocator<Eigen::MatrixXd> > empty_data;

    return write(surface, filename, 0, 0, empty_names, empty_data);
}