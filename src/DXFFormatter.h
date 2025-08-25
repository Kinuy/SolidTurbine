#pragma once

#include <string>
#include "IDXFWriter.h"
#include "DXFColor.h"

/**
 * @brief Static utility class for DXF file structure and formatting
 *
 * Provides methods for writing standard DXF file sections and headers.
 * Encapsulates DXF format knowledge and ensures consistency across
 * the export process. Follows Single Responsibility Principle.
 *
 * @note All methods are static - no instance required
 * @note Generates AutoCAD 2000 compatible DXF format
 */
class DXFFormatter {


public:

    /**
     * @brief Writes DXF file header section
     * @param writer Output writer to use
     *
     * Writes the HEADER section containing DXF version info and drawing
     * extents. Uses AutoCAD 2000 format (AC1015) for broad compatibility.
     */
    static void writeHeader(IDXFWriter& writer);

    /**
     * @brief Writes DXF tables section with default layer
     * @param writer Output writer to use
     *
     * Creates the TABLES section containing layer definitions.
     * Includes a default layer "0" with standard properties.
     */
    static void writeTables(IDXFWriter& writer);

    /**
     * @brief Writes empty DXF blocks section
     * @param writer Output writer to use
     *
     * Creates an empty BLOCKS section required by DXF specification.
     */
    static void writeBlocks(IDXFWriter& writer);

    /**
     * @brief Starts the DXF entities section
     * @param writer Output writer to use
     *
     * Begins the ENTITIES section where geometric entities are written.
     */
    static void startEntities(IDXFWriter& writer);

    /**
     * @brief Ends the DXF entities section
     * @param writer Output writer to use
     */
    static void endEntities(IDXFWriter& writer);

    /**
     * @brief Writes DXF end-of-file marker
     * @param writer Output writer to use
     */
    static void writeEOF(IDXFWriter& writer);

    /**
     * @brief Writes common entity header information
     * @param writer Output writer to use
     * @param entityType DXF entity type name
     * @param handle Unique entity handle
     * @param color Entity color
     *
     * Writes the standard header information that all DXF entities require
     * including type, handle, layer, and color information.
     */
    static void writeEntityHeader(IDXFWriter& writer, const std::string& entityType,
        const std::string& handle, const DXFColor& color);

};