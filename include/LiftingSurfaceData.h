#pragma once

#include "SurfaceData.h"
#include "LiftingSurface.h"
#include "Wake.h"

/**
 * @brief Store for all blade surface data and wake data
 */
struct LiftingSurfaceData : public SurfaceData{
    /**
    * @brief Constructs a LiftingSurfaceData object.
    * @param   lifting_surface   Lifting surface object.
    * @param   wake              Wake object for this surface.
    */
    LiftingSurfaceData(std::shared_ptr<LiftingSurface> liftingSurface, std::shared_ptr<Wake> wake) :
        SurfaceData(liftingSurface), liftingSurface(liftingSurface), wake(wake) {
    }

    /**
    * @brief Associated lifting surface object.
    */
    std::shared_ptr<LiftingSurface> liftingSurface;

    /**
    * @brief Associated wake object.
    */
    std::shared_ptr<Wake> wake;
};
