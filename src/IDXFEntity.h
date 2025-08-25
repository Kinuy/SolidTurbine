#pragma once

#include <string>
#include <vector>
#include "IDXFWriter.h"

/**
 * @brief Interface for DXF entities that can write themselves to output
 *
 * Enables polymorphic handling of different entity types. Each entity
 * knows how to serialize itself to DXF format. Supports Open/Closed
 * Principle by allowing new entity types without modifying existing code.
 */
class IDXFEntity {

public:


    virtual ~IDXFEntity() = default;

    /**
     * @brief Writes the entity to DXF format using the provided writer
     * @param writer Output writer interface
     * @param handleCounter Reference to handle counter for unique entity IDs
     */
    virtual void writeTo(IDXFWriter& writer, int& handleCounter) const = 0;

    /**
     * @brief Returns the DXF entity type name
     * @return String representing the DXF entity type (e.g., "LINE", "CIRCLE")
     */
    virtual std::string getEntityType() const = 0;

};

