#pragma once

#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief Interface for directory management operations
 *
 * IDirectoryManager provides an abstraction layer for filesystem directory
 * operations, enabling different directory management strategies through
 * polymorphism. Supports dependency injection for testing and platform
 * abstraction.
 *
 * ## Key Features
 * - **Platform Abstraction**: Isolates filesystem operations behind interface
 * - **Testability**: Enables mock implementations for unit testing
 * - **Strategy Pattern**: Different implementations for various requirements
 * - **Error Handling**: Boolean return values for simple success/failure checking
 *
 * ## Common Implementations
 * - **FileSystemDirectoryManager**: Standard filesystem operations
 * - **MockDirectoryManager**: Testing implementation
 * - **NetworkDirectoryManager**: Remote filesystem support
 *
 * @see FileSystemDirectoryManager for standard implementation
 * @see DataExporter for typical usage in export workflows
 *
 * @example
 * ```cpp
 * std::unique_ptr<IDirectoryManager> manager =
 *     std::make_unique<FileSystemDirectoryManager>();
 *
 * if (manager->ensureDirectoryExists("/output/data")) {
 *     // Directory ready for use
 * }
 * ```
 */
class IDirectoryManager {

public:

    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~IDirectoryManager() = default;

    /**
     * @brief Ensures specified directory exists, creating if necessary
     * @param path Directory path to verify/create
     * @return true if directory exists or was created successfully, false otherwise
     * @note Implementations should handle parent directory creation as needed
     */
    virtual bool ensureDirectoryExists(const fs::path& path) const = 0;

};