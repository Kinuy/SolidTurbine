#pragma once


#include <memory>
#include "Surface.h"
/**
 * @brief Struct to uniquely identify a panel edge on a surface
 */

struct SurfacePanelEdgeData {
public:
    /**
       @brief Constructor.
    */
    SurfacePanelEdgeData() {}

    /**
       @brief Constructor.
       @param   surface   Associated Surface object.
       @param   panel     Panel ID.
       @param   edge      Edge ID.
    */
    SurfacePanelEdgeData(std::shared_ptr<Surface> surface, int panel, int edge) : surface(surface), panel(panel), edge(edge) {};

    /**
       @brief Associated Surface object.
    */
    std::shared_ptr<Surface> surface;

    /**
       @brief Panel ID.
    */
    int panel;

    /**
       @brief Edge ID.
    */
    int edge;

    /**
       @brief Equality operator.
    */
    bool operator==(const SurfacePanelEdgeData& other) const
    {
        return (surface.get() == other.surface.get()) && (panel == other.panel) && (edge == other.edge);
    }
};
