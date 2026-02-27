#pragma once

#include <memory>
#include <vector>
#include <string>

#include "IStructuredData.h"

/**
 * @brief Interface for data file parsing strategies
 *
 * IDataFileParser defines the contract for parsing various data file formats
 * into structured data objects. Enables polymorphic file handling through
 * strategy pattern, allowing different parsers for different file types.
 *
 * ## Key Features
 * - **Format Agnostic**: Supports multiple file formats through implementations
 * - **Structured Output**: Converts raw files to typed data structures
 * - **Extension Validation**: Provides supported file extension information
 * - **Error Handling**: Implementations should provide clear error reporting
 *
 * ## Implementation Pattern
 * Each concrete parser handles specific file formats (blade geometry, airfoil
 * data, performance tables, etc.) and converts them to appropriate structured
 * data types implementing IStructuredData.
 *
 * @see IStructuredData for output data interface
 * @see BladeGeometryParser, AirfoilGeometryParser for concrete implementations
 *
 * @example
 * ```cpp
 * std::unique_ptr<IDataFileParser> parser =
 *     std::make_unique<BladeGeometryParser>();
 *
 * auto data = parser->parseFile("blade.dat");
 * auto extensions = parser->getSupportedExtensions();
 * ```
 */
class IDataFileParser {

public:

    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~IDataFileParser() = default;

    /**
     * @brief Parses data file into structured data object
     * @param filePath Path to file for parsing
     * @return Unique pointer to structured data containing parsed content
     * @throws std::runtime_error if file cannot be opened or parsed
     */
    virtual std::unique_ptr<IStructuredData> parseFile(const std::string& filePath) = 0;

    /**
     * @brief Gets list of file extensions supported by this parser
     * @return Vector of supported file extensions (e.g., {".dat", ".txt", ".csv"})
     */
    virtual std::vector<std::string> getSupportedExtensions() const = 0;

};