#pragma once

#include "Surface.h"

/**
* @brief
*/
class SurfaceWriter
{
public:

    /**
    * @brief
    */
    virtual ~SurfaceWriter() {};

    /**
    * @brief
    */
    virtual const char* fileExtension() const = 0;

    /**
    * @brief
    */
    bool write(
        const std::shared_ptr<Surface>& surface, 
        const std::string& filename);

    /**
    * @brief
    */
    bool write(
        const std::shared_ptr<Surface>& surface, 
        const std::string& filename,
        int node_offset, int panel_offset);

    /**
    * @brief
    */
    virtual bool write(
        const std::shared_ptr<Surface>& surface, 
        const std::string& filename,
        int node_offset, int panel_offset,
        const std::vector<std::string>& view_names, 
        const std::vector<Eigen::MatrixXd, Eigen::aligned_allocator<Eigen::MatrixXd> >& view_data) = 0;
};