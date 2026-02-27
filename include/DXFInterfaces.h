#pragma once

#include <string>
#include "DXFTypes.h"


/**
 * @brief Abstract writer interface for DXF output
 *
 * Defines the contract for writing DXF group code / value pairs.
 * Implementations can target files, streams, strings, etc.
 */
class IDXFWriter {
public:
    virtual ~IDXFWriter() = default;

    virtual void writeGroupCode(int code, const std::string& value) = 0;
    virtual void writeGroupCode(int code, double value) = 0;
    virtual void writeGroupCode(int code, int value) = 0;

    /** @brief Returns true if the writer is ready to accept data */
    virtual bool isOpen() const = 0;
};


/**
 * @brief Abstract interface for DXF entities
 *
 * All concrete DXF geometry types implement this interface to allow
 * uniform treatment in the document entity collection.
 */
class IDXFEntity {
public:
    virtual ~IDXFEntity() = default;

    /**
     * @brief Writes the entity to the given writer
     * @param writer  Output writer
     * @param handleCounter  Shared counter incremented per entity to produce unique handles
     */
    virtual void writeTo(IDXFWriter& writer, int& handleCounter) const = 0;

    /** @brief Returns the DXF type string, e.g. "LINE", "POINT" */
    virtual std::string getEntityType() const = 0;
};


/**
 * @brief Abstract interface for DXF handle generation
 *
 * Decouples handle generation strategy from the document,
 * enabling custom or test-specific handle sequences.
 */
class IDXFHandleGenerator {
public:
    virtual ~IDXFHandleGenerator() = default;

    /** @brief Returns the next unique handle string */
    virtual std::string getNextHandle() = 0;
};
