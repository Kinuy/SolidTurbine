#pragma once

#include "SurfacePanelEdgeData.h"

/**
* @brief Custom comparator that defines how to order, sort surface panel edge data objects with Priority: surface ID > panel > edge
*/
struct CompareSurfacePanelEdge {

    /**
    * @brief
    */
    bool operator()(const SurfacePanelEdgeData& a, const SurfacePanelEdgeData& b) const {
        if (a.surface->id != b.surface->id)
            return a.surface->id < b.surface->id;
        if (a.panel != b.panel)
            return a.panel < b.panel;
        return a.edge < b.edge;
    }
};