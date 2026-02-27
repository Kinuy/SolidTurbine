#pragma once

#include <memory>
#include <string>
#include <vector>

#include "IStructuredData.h"


/**
 * @brief Interface for parsing file list/index files
 *
 * IFileListParser defines the contract for parsing files that contain lists
 * or references to other files. Enables systematic processing of file collections
 * through index files, manifests, or directory listings.
 *
 * ## Key Features
 * - **Collection Processing**: Handles files containing lists of other files
 * - **Batch Operations**: Enables processing of multiple related files
 * - **Format Flexibility**: Supports various index file formats
 * - **Structured Output**: Converts file lists to organized data structures
 *
 * ## Typical Use Cases
 * - Airfoil geometry file collections
 * - Performance data file sets
 * - Batch processing workflows
 * - File manifest processing
 *
 * @see IDataFileParser for individual file parsing
 * @see AirfoilGeometryFileListParser, AirfoilPerformanceFileListParser for implementations
 *
 * @example
 * ```cpp
 * std::unique_ptr<IFileListParser> parser =
 *     std::make_unique<AirfoilGeometryFileListParser>();
 *
 * auto fileList = parser->parseFileList("airfoil_index.txt");
 * auto extensions = parser->getSupportedExtensions();
 * ```
 */
class IFileListParser {

public:

    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~IFileListParser() = default;

    /**
     * @brief Parses file list/index file into structured data
     * @param indexFilePath Path to index/manifest file containing file list
     * @return Unique pointer to structured data containing file references
     * @throws std::runtime_error if index file cannot be opened or parsed
     */
    virtual std::unique_ptr<IStructuredData> parseFileList(const std::string& indexFilePath) = 0;

    /**
     * @brief Gets list of file extensions supported by this parser
     * @return Vector of supported file extensions for index files
     */
    virtual std::vector<std::string> getSupportedExtensions() const = 0;

};