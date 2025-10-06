#pragma once

#include <memory>

#include "Surface.h"

/**
 * @brief Store for all surface data not including wake surfaces
 */
struct SurfaceData{

    /**
	 * @brief Constructor
	 * @param surface Pointer to the surface object
     */
    SurfaceData(std::shared_ptr<Surface> surface) :
        surface(surface) {
    }

    /**
    * @brief
    */
    std::shared_ptr<Surface> surface;

};
