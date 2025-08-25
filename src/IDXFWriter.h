#pragma once

#include <string>
/**
 * @brief Interface for writing DXF group codes to output destination
 *
 * Abstracts the writing mechanism allowing for different output targets
 * (files, memory, network, etc.). Follows Interface Segregation Principle
 * by providing only essential writing operations.
 *
 * @note Implementations must handle proper DXF formatting with newlines
 */
class IDXFWriter {

public:


    virtual ~IDXFWriter() = default;

    /**
     * @brief Writes a DXF group code with string value
     * @param code DXF group code number
     * @param value String value to write
     */
    virtual void writeGroupCode(int code, const std::string& value) = 0;

    /**
     * @brief Writes a DXF group code with double value
     * @param code DXF group code number
     * @param value Double value to write
     */
    virtual void writeGroupCode(int code, double value) = 0;

    /**
     * @brief Writes a DXF group code with integer value
     * @param code DXF group code number
     * @param value Integer value to write
     */
    virtual void writeGroupCode(int code, int value) = 0;

    /**
     * @brief Checks if the writer is ready for output
     * @return true if writer is open and ready, false otherwise
     */
    virtual bool isOpen() const = 0;

};