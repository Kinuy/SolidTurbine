#pragma once

#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "IDirectoryManager.h"

namespace fs = std::filesystem;

/**
 * @brief Standard filesystem directory management implementation
 *
 * FileSystemDirectoryManager implements IDirectoryManager using std::filesystem
 * for reliable cross-platform directory operations. Provides automatic directory
 * creation with comprehensive error handling and console logging.
 *
 * ## Key Features
 * - **Recursive Creation**: Creates parent directories as needed
 * - **Error Handling**: Graceful handling of filesystem errors
 * - **Console Logging**: Success/failure reporting to appropriate streams
 * - **Cross-Platform**: Uses std::filesystem for portability
 *
 * ## Use Cases
 * - Output directory preparation for file writers
 * - Build system directory management
 * - Data export pipeline setup
 *
 * @see IDirectoryManager for the base interface
 * @see DataExporter for typical usage in export workflows
 *
 * @example
 * ```cpp
 * auto manager = std::make_unique<FileSystemDirectoryManager>();
 * bool success = manager->ensureDirectoryExists("output/results");
 * ```
 */
class FileSystemDirectoryManager : public IDirectoryManager {

public:

    /**
     * @brief Ensures directory exists, creating it if necessary
     *
     * Checks if the specified directory path exists and creates it (including
     * parent directories) if it doesn't. Provides console logging for both
     * creation events and error conditions.
     *
     * @param path Directory path to verify/create
     * @return true if directory exists or was created successfully, false on error
     *
     * @note Uses std::filesystem::create_directories for recursive creation
     * @note Logs creation events to stdout, errors to stderr
     *
     * @example
     * ```cpp
     * FileSystemDirectoryManager manager;
     * if (manager.ensureDirectoryExists("/output/data/results")) {
     *     // Directory ready for use
     * }
     * ```
     */
    bool ensureDirectoryExists(const fs::path& path) const override;
};
