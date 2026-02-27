#pragma once

#include <string>
#include <vector>
#include <memory>
#include "DXFTypes.h"
#include "DXFInterfaces.h"
#include "DXFCore.h"


// ============================================================
//  DXFEntityBase
// ============================================================

/**
 * @brief Abstract base class for all DXF entities
 *
 * Holds the entity color and provides the shared writeEntityHeader() helper.
 * Concrete entity classes inherit from this and implement writeTo() and
 * getEntityType().
 */
class DXFEntityBase : public IDXFEntity {
protected:
    /** @brief Entity color using AutoCAD Color Index */
    DXFColor color;

    /**
     * @brief Writes the standard entity header via DXFFormatter
     * @param writer  Output writer
     * @param handle  Unique handle string
     */
    void writeEntityHeader(IDXFWriter& writer, const std::string& handle) const;

public:
    explicit DXFEntityBase(const DXFColor& c = DXFColor());
};


// ============================================================
//  DXFLine
// ============================================================

/**
 * @brief DXF LINE entity — straight segment between two 3D points
 */
class DXFLine : public DXFEntityBase {
private:
    DXFPoint3D startPoint;
    DXFPoint3D endPoint;

public:
    DXFLine(const DXFPoint3D& start, const DXFPoint3D& end, const DXFColor& color = DXFColor());

    void writeTo(IDXFWriter& writer, int& handleCounter) const override;
    std::string getEntityType() const override;
};


// ============================================================
//  DXFPoint
// ============================================================

/**
 * @brief DXF POINT entity — single point in 3D space
 */
class DXFPoint : public DXFEntityBase {
private:
    DXFPoint3D position;

public:
    DXFPoint(const DXFPoint3D& pos, const DXFColor& color = DXFColor());

    void writeTo(IDXFWriter& writer, int& handleCounter) const override;
    std::string getEntityType() const override;
};


// ============================================================
//  DXFPolyLine
// ============================================================

/**
 * @brief DXF LWPOLYLINE entity — connected line segments, open or closed
 *
 * Uses 2D coordinates per DXF LWPOLYLINE spec; Z values from DXFPoint3D
 * are written but may be ignored by some CAD applications.
 * For true 3D polylines, a separate DXF3DPolyLine entity would be needed.
 */
class DXFPolyLine : public DXFEntityBase {
private:
    std::vector<DXFPoint3D> points;
    bool closed;

public:
    DXFPolyLine(const std::vector<DXFPoint3D>& pts, bool isClosed = false, const DXFColor& color = DXFColor());

    void writeTo(IDXFWriter& writer, int& handleCounter) const override;
    std::string getEntityType() const override;
};


// ============================================================
//  DXFText
// ============================================================

/**
 * @brief DXF TEXT entity — single-line text at a 3D insertion point
 */
class DXFText : public DXFEntityBase {
private:
    DXFPoint3D  position;
    std::string text;
    double      height;

public:
    DXFText(const DXFPoint3D& pos, const std::string& txt, double h = 1.0, const DXFColor& color = DXFColor());

    void writeTo(IDXFWriter& writer, int& handleCounter) const override;
    std::string getEntityType() const override;
};


// ============================================================
//  DXFEntityFactory
// ============================================================

/**
 * @brief Factory for creating DXF entity instances via smart pointers
 *
 * Hides concrete entity types from calling code. All methods are static.
 * Extend here when adding new entity types.
 */
class DXFEntityFactory {
public:
    /** @brief Creates a LINE entity */
    static std::unique_ptr<IDXFEntity> createLine(
        const DXFPoint3D& start, const DXFPoint3D& end,
        const DXFColor& color = DXFColor());

    /** @brief Creates a LWPOLYLINE entity */
    static std::unique_ptr<IDXFEntity> createPolyLine(
        const std::vector<DXFPoint3D>& points, bool closed = false,
        const DXFColor& color = DXFColor());

    /** @brief Creates a TEXT entity */
    static std::unique_ptr<IDXFEntity> createText(
        const DXFPoint3D& position, const std::string& text,
        double height = 1.0, const DXFColor& color = DXFColor());

    /** @brief Creates a POINT entity */
    static std::unique_ptr<IDXFEntity> createPoint(
        const DXFPoint3D& position, const DXFColor& color = DXFColor());

    /**
     * @brief Creates a closed rectangular LWPOLYLINE from two opposite corners
     * @param corner1  One corner of the rectangle
     * @param corner2  Opposite corner of the rectangle
     */
    static std::unique_ptr<IDXFEntity> createRectangle(
        const DXFPoint3D& corner1, const DXFPoint3D& corner2,
        const DXFColor& color = DXFColor());
};
