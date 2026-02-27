#pragma once
#include <fstream>
#include "SurfaceWriter.h"

/**
* @brief
*/
class VTKFileWriter : public SurfaceWriter
{
public:

    /**
    * @brief
    */
    const char* fileExtension() const;

    /**
    * @brief
    */
    bool write(
        const std::shared_ptr<Surface>& surface, 
        const std::string& filename,
        int node_offset, int panel_offset,
        const std::vector<std::string>& view_names, 
        const std::vector<Eigen::MatrixXd, Eigen::aligned_allocator<Eigen::MatrixXd> >& view_data);
};