#pragma once


#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <memory>

#include "IFileListParser.h"
#include "IStructuredData.h"
#include "AirfoilPerformanceFileListData.h"

/**
 * @brief File list parser implementation for airfoil performance file collections
 *
 * AirfoilPerformanceFileListParser implements IFileListParser to read and process
 * index files containing lists of airfoil performance files. Handles path resolution,
 * file validation, and creates structured data for batch processing of airfoil
 * performance datasets.
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
 * - **Extensions**: .txt, .dat, .list, .files for index files
 * - **Path Types**: Both absolute and relative paths supported
 *
 * ## Typical Index File
 * ```
 * # Airfoil performance file collection
 * airfoils/naca0012.dat
 * airfoils/naca0015.dat
 * /absolute/path/to/custom_airfoil.perf
 * ```
 *
 * ## Use Cases
 * - Batch processing of multiple airfoil performance files
 * - Airfoil database organization and management
 * - Systematic airfoil performance analysis workflows
 *
 * @see IFileListParser for the base interface
 * @see AirfoilPerformanceFileListData for output data structure
 * @see AirfoilPerformanceParser for individual file parsing
 *
 * @example
 * ```cpp
 * AirfoilPerformanceFileListParser parser;
 * auto fileList = parser.parseFileList("airfoil_index.txt");
 * auto listData = static_cast<AirfoilPerformanceFileListData*>(fileList.get());
 * ```
 */
class AirfoilPerformanceFileListParser : public IFileListParser {

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
     * @brief Parses file list index into structured data with path validation
     * @param indexFilePath Path to index file containing list of performance files
     * @return Unique pointer to AirfoilPerformanceFileListData with resolved file paths
     * @throws std::runtime_error if file cannot be opened, no paths found, or no valid files exist
     * @note Validates file existence and provides detailed error messages for missing files
     */
    std::unique_ptr<IStructuredData> parseFileList(const std::string& indexFilePath) override;

    /**
     * @brief Gets list of supported file extensions for index files
     * @return Vector of supported extensions: .txt, .dat, .list, .files
     */
    std::vector<std::string> getSupportedExtensions() const override;

};

