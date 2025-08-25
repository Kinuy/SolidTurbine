#pragma once


#include "DXFEntityBase.h"
#include "DXFPoint3D.h"
#include "DXFColor.h"

/**
 * @brief DXF POINT entity implementation
 *
 * Represents a single point in 3D space. Point display style depends
 * on the CAD application's point display settings.
 */
class DXFPoint : public DXFEntityBase {


private:

    /**
     * @brief Point location in 3D space
     */
    DXFPoint3D position;

public:

    /**
     * @brief Constructs a point entity
     * @param pos Point location in 3D space
     * @param color Point color (default: white/black)
     */
    DXFPoint(const DXFPoint3D& pos, const DXFColor& color = DXFColor());

    /**
     * @brief Writes point entity to DXF format
     * @param writer Output writer interface
     * @param handleCounter Reference to handle counter for unique entity ID
     */
    void writeTo(IDXFWriter& writer, int& handleCounter) const override;

    /**
     * @brief Returns DXF entity type identifier
     * @return "POINT" as required by DXF specification
     */
    std::string getEntityType() const override;

};

