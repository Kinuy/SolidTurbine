#pragma once

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include "IFileListParser.h"
#include "AirfoilGeometryFileListData.h"
#include "IStructuredData.h"


/**
 * @brief File list parser implementation for airfoil geometry file collections
 *
 * AirfoilGeometryFileListParser implements IFileListParser to read and process
 * index files containing lists of airfoil geometry files. Handles path resolution,
 * file validation, and creates structured data for batch processing of airfoil
 * geometry datasets.
 *
 * ## Key Features
 * - **Path Resolution**: Converts relative paths to absolute based on index file location
 * - **File Validation**: Checks existence of referenced files and reports missing ones
 * - **Flexible Format**: Supports various index file formats with comment lines
 * - **Error Reporting**: Detailed error messages for missing or invalid files
 *
 * ## Supported Index Format
 * - **File Paths**: One file path per line (absolute or relative)
 * - **Comments**: Lines starting with '#' (preserved as headers)
 * - **Extensions**: .txt, .dat, .list, .files, .geo for index files
 * - **Path Types**: Both absolute and relative paths supported
 *
 * ## Typical Index File
 * ```
 * # Airfoil geometry file collection
 * airfoils/naca0012.dat
 * airfoils/naca0015.geo
 * /absolute/path/to/custom_airfoil.coord
 * ```
 *
 * ## Use Cases
 * - Batch processing of multiple airfoil geometry files
 * - Airfoil geometry database organization and management
 * - Systematic blade design workflows with multiple airfoil sections
 * - CFD preprocessing with airfoil coordinate collections
 *
 * @see IFileListParser for the base interface
 * @see AirfoilGeometryFileListData for output data structure
 * @see AirfoilGeometryParser for individual file parsing
 *
 * @example
 * ```cpp
 * AirfoilGeometryFileListParser parser;
 * auto fileList = parser.parseFileList("geometry_index.txt");
 * auto listData = static_cast<AirfoilGeometryFileListData*>(fileList.get());
 * ```
 */
class AirfoilGeometryFileListParser : public IFileListParser {

private:

    /**
     * @brief Trims whitespace from both ends of a string
     * @param line Input string to trim
     * @return String with leading/trailing whitespace removed
     */
    std::string trimLine(const std::string& line) const;

    /**
     * @brief Checks if path is absolute
     * @param path File path to check
     * @return true if path is absolute, false if relative
     */
    bool isAbsolutePath(const std::string& path) const;

    /**
     * @brief Resolves relative path to absolute path based on index file location
     * @param relativePath Path to resolve (returned unchanged if already absolute)
     * @param indexFilePath Path to index file (used as base directory for resolution)
     * @return Absolute path string
     */
    std::string resolveRelativePath(const std::string& relativePath, const std::string& indexFilePath) const;

public:

    /**
     * @brief Parses geometry file list index into structured data with path validation
     * @param indexFilePath Path to index file containing list of geometry files
     * @return Unique pointer to AirfoilGeometryFileListData with resolved file paths
     * @throws std::runtime_error if file cannot be opened, no paths found, or no valid files exist
     * @note Validates file existence and provides detailed error messages for missing files
     */
    std::unique_ptr<IStructuredData> parseFileList(const std::string& indexFilePath) override;

    /**
     * @brief Gets list of supported file extensions for index files
     * @return Vector of supported extensions: .txt, .dat, .list, .files, .geo
     */
    std::vector<std::string> getSupportedExtensions() const override;

};

