#pragma once

#include "IDXFWriter.h"
#include <fstream>
#include <stdexcept>
#include <string>


/**
 * @brief File-based implementation of IDXFWriter interface
 *
 * Handles writing DXF data directly to a file on disk. Manages file
 * lifecycle and ensures proper formatting for DXF specification compliance.
 * Follows Single Responsibility Principle by focusing only on file I/O.
 *
 * @note Automatically closes file in destructor
 * @note Throws runtime_error if file cannot be opened
 */
class DXFFileWriter : public IDXFWriter {


private:

    /**
     * @brief Output file stream
     */
    std::ofstream file;

public:

    /**
     * @brief Constructs file writer and opens output file
     * @param filename Path to output DXF file
     * @throws std::runtime_error if file cannot be opened
     */
    explicit DXFFileWriter(const std::string& filename);

    /**
     * @brief Destructor - automatically closes file if open
     */
    ~DXFFileWriter();

    /**
     * @brief Writes group code and string value in DXF format
     * @param code DXF group code
     * @param value String value to write
     */
    void writeGroupCode(int code, const std::string& value) override;

    /**
     * @brief Writes group code and double value in DXF format
     * @param code DXF group code
     * @param value Double value to write
     */
    void writeGroupCode(int code, double value) override;

    /**
     * @brief Writes group code and integer value in DXF format
     * @param code DXF group code
     * @param value Integer value to write
     */
    void writeGroupCode(int code, int value) override;

    /**
     * @brief Checks if file is open and ready for writing
     * @return true if file is open, false otherwise
     */
    bool isOpen() const override;
};

