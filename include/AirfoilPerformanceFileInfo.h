#pragma once

#include <string>
#include <filesystem>

/**
 * @brief Information structure for individual airfoil performance files
 *
 * AirfoilPerformanceFileInfo stores metadata about a single airfoil performance
 * file including its path, extracted filename, and filesystem existence status.
 * Automatically validates file existence during construction.
 *
 * ## Key Features
 * - **Automatic Validation**: Checks file existence during construction
 * - **Path Processing**: Extracts filename from full path
 * - **Status Tracking**: Maintains existence flag for collection validation
 *
 * ## Usage Context
 * Used within AirfoilPerformanceFileListData to track individual files
 * in airfoil performance collections with validation status.
 *
 * @see AirfoilPerformanceFileListData for collection management
 * @see std::filesystem for path operations
 *
 * @example
 * ```cpp
 * AirfoilPerformanceFileInfo info("/path/to/naca0012.dat");
 * // info.fileName = "naca0012.dat"
 * // info.exists = true/false based on filesystem check
 * ```
 */
struct AirfoilPerformanceFileInfo {

    /**
     * @brief Full path to the airfoil performance file
     */
    std::string filePath;

    /**
     * @brief Extracted filename without directory path
     */
    std::string fileName;

    /**
     * @brief True if file exists on filesystem at construction time
     */
    bool exists;

    /**
     * @brief Constructor with automatic filename extraction and existence validation
     * @param path Full path to airfoil performance file
     * @note Automatically extracts filename and checks filesystem existence
     */
    AirfoilPerformanceFileInfo(const std::string& path) : filePath(path) {
        fileName = std::filesystem::path(path).filename().string();
        exists = std::filesystem::exists(path);
    }

};