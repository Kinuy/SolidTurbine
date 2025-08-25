#pragma once

#include <string>

#include "DXFEntityBase.h"
#include "DXFPoint3D.h"
#include "DXFColor.h"
#include "IDXFWriter.h"


/**
 * @brief DXF LINE entity implementation
 *
 * Represents a straight line segment between two 3D points.
 * Follows Single Responsibility Principle by handling only line-specific data.
 *
 * @note Lines are always straight - use DXFPolyline for multi-segment paths
 */
class DXFLine : public DXFEntityBase {


private:

    /**
     * @brief Line start point
     */
    DXFPoint3D startPoint;

    /**
	 * @brief Line end point
     */
    DXFPoint3D endPoint;

public:

    /**
     * @brief Constructs a line entity
     * @param start Starting point of the line
     * @param end Ending point of the line
     * @param color Line color (default: white/black)
     */
    DXFLine(const DXFPoint3D& start, const DXFPoint3D& end, const DXFColor& color = DXFColor());

    /**
     * @brief Writes line entity to DXF format
     * @param writer Output writer interface
     * @param handleCounter Reference to handle counter for unique entity ID
     */
    void writeTo(IDXFWriter& writer, int& handleCounter) const override;

    /**
     * @brief Returns DXF entity type identifier
     * @return "LINE" as required by DXF specification
     */
    std::string getEntityType() const override;

};

