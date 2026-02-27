#pragma once

#include <string>
#include <fstream>
#include <stdexcept>
#include "DXFTypes.h"
#include "DXFInterfaces.h"


// ============================================================
//  DXFFormatter
// ============================================================

/**
 * @brief Static utility class for DXF file structure and formatting
 *
 * Writes standard DXF file sections (HEADER, TABLES, BLOCKS, ENTITIES)
 * and common entity headers. All methods are static — no instance required.
 * Generates AutoCAD 2000 compatible DXF format (AC1015).
 */
class DXFFormatter {
public:
    /** @brief Writes HEADER section (DXF version, drawing extents) */
    static void writeHeader(IDXFWriter& writer);

    /** @brief Writes TABLES section with a default layer "0" */
    static void writeTables(IDXFWriter& writer);

    /** @brief Writes empty BLOCKS section (required by spec) */
    static void writeBlocks(IDXFWriter& writer);

    /** @brief Opens the ENTITIES section */
    static void startEntities(IDXFWriter& writer);

    /** @brief Closes the ENTITIES section */
    static void endEntities(IDXFWriter& writer);

    /** @brief Writes the EOF marker */
    static void writeEOF(IDXFWriter& writer);

    /**
     * @brief Writes the common entity header (type, handle, layer, color)
     * @param writer       Output writer
     * @param entityType   DXF type string, e.g. "LINE"
     * @param handle       Unique entity handle string
     * @param color        Entity color
     */
    static void writeEntityHeader(IDXFWriter& writer, const std::string& entityType,
                                  const std::string& handle, const DXFColor& color);
};


// ============================================================
//  DXFHandleGenerator
// ============================================================

/**
 * @brief Sequential handle generator for DXF entities
 *
 * Generates unique sequential string handles starting from a configurable value.
 */
class DXFHandleGenerator : public IDXFHandleGenerator {
private:
    int currentHandle;

public:
    /** @brief Constructs generator with given start value (default: 100) */
    explicit DXFHandleGenerator(int startHandle = 100);

    /** @brief Returns the next handle and advances the counter */
    std::string getNextHandle() override;
};


// ============================================================
//  DXFFileWriter
// ============================================================

/**
 * @brief File-based implementation of IDXFWriter
 *
 * Writes DXF group code / value pairs directly to a file on disk.
 * Throws std::runtime_error if the file cannot be opened.
 * File is closed automatically in the destructor.
 */
class DXFFileWriter : public IDXFWriter {
private:
    std::ofstream file;

public:
    /**
     * @brief Opens the output file
     * @throws std::runtime_error if file cannot be opened
     */
    explicit DXFFileWriter(const std::string& filename);

    ~DXFFileWriter();

    void writeGroupCode(int code, const std::string& value) override;
    void writeGroupCode(int code, double value) override;
    void writeGroupCode(int code, int value) override;

    bool isOpen() const override;
};
