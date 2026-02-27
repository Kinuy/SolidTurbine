#pragma once

#include <string>
#include <vector>
#include <filesystem>

/**
 * @brief Information structure for individual airfoil geometry files
 *
 * AirfoilGeometryFileInfo stores metadata about a single airfoil geometry
 * file including its path, extracted filename, and filesystem existence status.
 * Automatically validates file existence during construction.
 *
 * ## Key Features
 * - **Automatic Validation**: Checks file existence during construction
 * - **Path Processing**: Extracts filename from full path
 * - **Status Tracking**: Maintains existence flag for collection validation
 *
 * ## Usage Context
 * Used within AirfoilGeometryFileListData to track individual geometry files
 * in airfoil coordinate collections with validation status.
 *
 * @see AirfoilGeometryFileListData for collection management
 * @see std::filesystem for path operations
 *
 * @example
 * ```cpp
 * AirfoilGeometryFileInfo info("/path/to/naca0012.dat");
 * // info.fileName = "naca0012.dat"
 * // info.exists = true/false based on filesystem check
 * ```
 */
struct AirfoilGeometryFileInfo {

    /**
     * @brief Full path to the airfoil geometry file
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
     * @param path Full path to airfoil geometry file
     * @note Automatically extracts filename and checks filesystem existence
     */
    AirfoilGeometryFileInfo(const std::string& path) : filePath(path) {
        fileName = std::filesystem::path(path).filename().string();
        exists = std::filesystem::exists(path);
    }

};