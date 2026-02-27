#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>

#include "IStructuredData.h"
#include "AirfoilGeometryFileInfo.h"
#include "AirfoilGeometryFileListParser.h"


/**
 * @brief Structured data container for airfoil geometry file collections
 *
 * AirfoilGeometryFileListData implements IStructuredData to store and manage
 * collections of airfoil geometry files with metadata. Tracks file existence,
 * extracts revision/date information, and provides convenient access to valid
 * file subsets for batch processing workflows.
 *
 * ## Key Features
 * - **File Validation**: Tracks existence status for each referenced geometry file
 * - **Metadata Extraction**: Automatically parses revision and date from headers
 * - **Filtered Access**: Provides methods to get only valid/missing files
 * - **Name-Based Search**: Enables lookup of geometry files by airfoil name
 *
 * ## Use Cases
 * - Managing large airfoil geometry databases for blade design
 * - Batch processing workflows for multiple geometry files
 * - Validation and reporting of geometry file collection integrity
 * - Systematic airfoil coordinate processing for CFD and analysis
 *
 * @see IStructuredData for the base interface
 * @see AirfoilGeometryFileInfo for individual file information
 * @see AirfoilGeometryFileListParser for file list parsing
 *
 * @example
 * ```cpp
 * AirfoilGeometryFileListData geometryList;
 * geometryList.addFilePath("naca0012.dat");
 * auto validPaths = geometryList.getValidFilePaths();
 * auto naca0012Info = geometryList.getFileByName("naca0012");
 * ```
 */
class AirfoilGeometryFileListData : public IStructuredData {

private:

    /**
     * @brief Collection of geometry file information objects with existence validation
     */
    std::vector<AirfoilGeometryFileInfo> fileInfos;

    /**
     * @brief Header lines from the file list (comments and metadata)
     */
    std::vector<std::string> headers;

    /**
     * @brief Revision information extracted from headers
     */
    std::string revision;

    /**
     * @brief Date information extracted from headers
     */
    std::string date;

public:

    /**
     * @brief Adds header line and extracts special metadata
     * @param header Header string to add (automatically parses Revision and Date)
     */
    void addHeader(const std::string& header);

    /**
     * @brief Adds file path to the collection with existence validation
     * @param filePath Path to airfoil geometry file
     */
    void addFilePath(const std::string& filePath);

    /**
     * @brief Gets all file information entries
     * @return Const reference to vector of AirfoilGeometryFileInfo objects
     */
    const std::vector<AirfoilGeometryFileInfo>& getFileInfos() const;

    /**
     * @brief Gets all stored header lines
     * @return Const reference to vector of header strings
     */
    const std::vector<std::string>& getHeaders() const;

    /**
     * @brief Gets extracted revision information
     * @return Revision string parsed from headers
     */
    const std::string& getRevision() const;

    /**
     * @brief Gets extracted date information
     * @return Date string parsed from headers
     */
    const std::string& getDate() const;

    /**
     * @brief Gets the data type identifier
     * @return Always returns "AirfoilGeometryFileList"
     */
    std::string getTypeName() const override;

    /**
     * @brief Gets the number of file entries
     * @return Total number of files in the list
     */
    size_t getRowCount() const override;

    /**
     * @brief Gets paths of files that exist on filesystem
     * @return Vector of valid file path strings
     */
    std::vector<std::string> getValidFilePaths() const;

    /**
     * @brief Gets paths of files that don't exist on filesystem
     * @return Vector of missing file path strings
     */
    std::vector<std::string> getMissingFiles() const;

    /**
     * @brief Gets count of files that exist on filesystem
     * @return Number of valid/existing files
     */
    size_t getValidFileCount() const;

    /**
     * @brief Finds geometry file by airfoil name search
     * @param airfoilName Name/identifier to search for in filenames
     * @return AirfoilGeometryFileInfo for matching file
     * @throws std::runtime_error if no matching file found
     */
    AirfoilGeometryFileInfo getFileByName(const std::string& airfoilName) const;

};

