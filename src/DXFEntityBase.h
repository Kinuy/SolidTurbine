#pragma once


#include <string>
#include "IDXFEntity.h"
#include "DXFColor.h"
#include "IDXFWriter.h"
#include "DXFFormatter.h"


/**
 * @brief Abstract base class for all DXF entities
 *
 * Provides common functionality for DXF entities including color management
 * and entity header writing. Enables code reuse while maintaining the
 * Open/Closed Principle for entity extensions.
 */
class DXFEntityBase : public IDXFEntity {


protected:

    /**
     * @brief Entity color using AutoCAD DXFColor Index
     */
    DXFColor color;

public:

    /**
     * @brief Constructs base entity with color
     * @param c Entity color (default: DXFColor(7) - white/black)
     */
    explicit DXFEntityBase(const DXFColor& c = DXFColor());

    /**
     * @brief Writes standard entity header to DXF output
     * @param writer Output writer interface
     * @param handle Unique entity handle
     *
     * Helper method that writes the common header information
     * shared by all DXF entities.
     */
    void writeEntityHeader(IDXFWriter& writer, const std::string& handle) const;

};
