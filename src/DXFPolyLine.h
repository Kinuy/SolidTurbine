#pragma once


#include <vector>
#include <string>
#include "DXFEntityBase.h"
#include "DXFPoint3D.h"
#include "IDXFWriter.h"
#include "DXFColor.h"




/**
 * @brief DXF lightweight polyline entity implementation
 *
 * Represents a sequence of connected line segments that can be open or closed.
 * Uses 2D coordinates (Z values from points are ignored). For 3D polylines,
 * consider implementing DXF3DPolyline separately.
 */
class DXFPolyLine : public DXFEntityBase {


private:

    /**
     * @brief Polyline vertices
     */
    std::vector<DXFPoint3D> points;

    /**
     * @brief Whether polyline is closed (forms polygon)
     */
    bool closed;

public:

    /**
     * @brief Constructs a polyline entity
     * @param pts Vector of polyline vertices (minimum 2 points)
     * @param isClosed Whether to close the polyline (connect last to first point)
     * @param color Polyline color (default: white/black)
     */
    DXFPolyLine(const std::vector<DXFPoint3D>& pts, bool isClosed = false, const DXFColor& color = DXFColor());

    /**
     * @brief Writes polyline entity to DXF format
     * @param writer Output writer interface
     * @param handleCounter Reference to handle counter for unique entity ID
     */
    void writeTo(IDXFWriter& writer, int& handleCounter) const override;

    /**
     * @brief Returns DXF entity type identifier
     * @return "LWPOLYLINE" as required by DXF specification
     */
    std::string getEntityType() const override;

};

