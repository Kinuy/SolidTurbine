#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "IStructuredData.h"
#include "AirfoilPerformanceFileInfo.h"


/**
 * @brief Structured data container for airfoil performance file collections
 *
 * AirfoilPerformanceFileListData implements IStructuredData to store and manage
 * collections of airfoil performance files with metadata. Tracks file existence,
 * extracts revision/date information, and provides convenient access to valid
 * file subsets for batch processing workflows.
 *
 * ## Key Features
 * - **File Validation**: Tracks existence status for each referenced file
 * - **Metadata Extraction**: Automatically parses revision and date from headers
 * - **Filtered Access**: Provides methods to get only valid/missing files
 * - **Batch Processing**: Enables systematic processing of airfoil collections
 *
 * ## Use Cases
 * - Managing large airfoil performance databases
 * - Batch processing workflows for multiple airfoil files
 * - Validation and reporting of file collection integrity
 * - Organized access to airfoil performance datasets
 *
 * @see IStructuredData for the base interface
 * @see AirfoilPerformanceFileInfo for individual file information
 * @see AirfoilPerformanceFileListParser for file list parsing
 *
 * @example
 * ```cpp
 * AirfoilPerformanceFileListData fileList;
 * fileList.addFilePath("naca0012.dat");
 * auto validPaths = fileList.getValidFilePaths();
 * auto missingFiles = fileList.getMissingFiles();
 * ```
 */
class AirfoilPerformanceFileListData : public IStructuredData {

private:

    /**
     * @brief Collection of file information objects with existence validation
     */
    std::vector<AirfoilPerformanceFileInfo> fileInfos;

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
     * @param filePath Path to airfoil performance file
     */
    void addFilePath(const std::string& filePath);

    /**
     * @brief Gets all file information entries
     * @return Const reference to vector of AirfoilPerformanceFileInfo objects
     */
    const std::vector<AirfoilPerformanceFileInfo>& getFileInfos() const;

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
     * @brief Gets extracted date information
     * @return Date string parsed from headers
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

};

