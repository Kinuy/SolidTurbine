#pragma once


#include <memory>
#include <vector>
#include <string>
#include "IDXFEntity.h"
#include "DXFPoint3D.h"
#include "DXFColor.h"
#include "DXFLine.h"
#include "DXFPolyLine.h"
#include "DXFText.h"
#include "DXFPoint.h"
#include "DXFFormatter.h"
#include "DXFEntityBase.h"
#include "DXFEntityFactory.h"
#include "DXFHandleGenerator.h"
#include "IDXFWriter.h"


/**
 * @brief Factory class for creating DXF entities using RAII and smart pointers
 *
 * Provides a clean interface for creating DXF entities without exposing
 * concrete classes to client code. Supports the Open/Closed Principle by
 * allowing new entity types to be added without modifying existing factory methods.
 *
 * @note All factory methods return std::unique_ptr for automatic memory management
 * @note Factory methods are static - no instance required
 */
class DXFEntityFactory {


public:

    /**
     * @brief Creates a line entity between two points
     * @param start Starting point of the line
     * @param end Ending point of the line
     * @param color Line color (default: white/black)
     * @return Unique pointer to created line entity
     */
    static std::unique_ptr<IDXFEntity> createLine(const DXFPoint3D& start, const DXFPoint3D& end, const DXFColor& color);

    /**
     * @brief Creates a polyline entity from a sequence of points
     * @param points Vector of polyline vertices (minimum 2 points recommended)
     * @param closed Whether to close the polyline (connect last to first point)
     * @param color Polyline color (default: white/black)
     * @return Unique pointer to created polyline entity
     */
    static std::unique_ptr<IDXFEntity> createPolyLine(const std::vector<DXFPoint3D>& points, bool closed, const DXFColor& color);

    /**
     * @brief Creates a text entity
     * @param position Text insertion point
     * @param text Text content string
     * @param height Text height in drawing units (default: 1.0)
     * @param color Text color (default: white/black)
     * @return Unique pointer to created text entity
     */
    static std::unique_ptr<IDXFEntity> createText(const DXFPoint3D& position, const std::string& text, double height, const DXFColor& color);

    /**
     * @brief Creates a point entity
     * @param position Point location in 3D space
     * @param color Point color (default: white/black)
     * @return Unique pointer to created point entity
     */
    static std::unique_ptr<IDXFEntity> createPoint(const DXFPoint3D& position, const DXFColor& color);

    /**
     * @brief Creates a rectangular polyline from two corner points
     * @param corner1 First corner of the rectangle
     * @param corner2 Opposite corner of the rectangle
     * @param color Rectangle color (default: white/black)
     * @return Unique pointer to created closed polyline forming rectangle
     *
     * @note Creates a closed polyline with 4 vertices forming a rectangle
     */
    static std::unique_ptr<IDXFEntity> createRectangle(const DXFPoint3D& corner1, const DXFPoint3D& corner2, const DXFColor& color);

};

