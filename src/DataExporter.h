#pragma once

#include <filesystem>
#include <string>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "IExporter.h"
#include "IPathResolver.h"
#include "IDirectoryManager.h"
#include "IFileWriter.h"

namespace fs = std::filesystem;


/**
 * @brief Concrete implementation of IExporter using dependency injection for flexible data export operations
 *
 * DataExporter provides a complete data export solution by orchestrating three key components:
 * path resolution, directory management, and file writing. This class follows the dependency
 * injection pattern, allowing different strategies and implementations to be plugged in at
 * runtime, making it highly testable and configurable.
 *
 * ## Architecture
 *
 * The DataExporter acts as a coordinator that delegates specific responsibilities to injected
 * dependencies:
 * - **Path Resolution**: Determines where files should be written based on input paths
 * - **Directory Management**: Ensures target directories exist and are accessible
 * - **File Writing**: Handles the actual file I/O operations
 *
 * ## Design Benefits
 *
 * - **Flexibility**: Different path resolution strategies can be used (default, flexible, etc.)
 * - **Testability**: Dependencies can be mocked for unit testing
 * - **Separation of Concerns**: Each component has a single, well-defined responsibility
 * - **Configurability**: Behavior can be changed by swapping implementations
 *
 * ## Usage Pattern
 *
 * This class is typically created through the ExporterFactory, which assembles the
 * appropriate dependencies based on configuration requirements. Direct instantiation
 * is possible but requires manual dependency management.
 *
 * @see IExporter for the interface definition and contract
 * @see ExporterFactory for factory-based creation with common configurations
 * @see IPathResolver for path resolution strategy interfaces
 * @see IDirectoryManager for directory management interfaces
 * @see IFileWriter for file writing strategy interfaces
 *
 * @note This class takes ownership of all injected dependencies and manages their
 *       lifetime through unique_ptr smart pointers
 *
 * @thread_safety This class is not thread-safe. External synchronization is required
 *                for concurrent access to the same DataExporter instance.
 *
 * @example
 * ```cpp
 * // Factory-based creation (recommended)
 * auto exporter = ExporterFactory::createExporter(
 *     PathStrategy::FLEXIBLE,
 *     "/default/output"
 * );
 *
 * // Export data
 * bool success = exporter->exportData("subdir", "data.json", jsonData);
 * ```
 */
class DataExporter : public IExporter {

private:

    /**
     * @brief Path resolution strategy for determining export directory locations
     *
     * Handles the conversion of input path strings to absolute filesystem paths
     * according to the configured resolution strategy. Different implementations
     * provide different behaviors for handling relative paths, empty paths, and
     * path validation.
     *
     * @see IPathResolver for available implementations and their behaviors
     */
    std::unique_ptr<IPathResolver> pathResolver;

    /**
     * @brief Directory management component for ensuring target directories exist
     *
     * Responsible for creating directories as needed and verifying that target
     * locations are accessible for writing. Abstracts platform-specific directory
     * operations and permission handling.
     *
     * @see IDirectoryManager for directory creation and validation operations
     */
    std::unique_ptr<IDirectoryManager> directoryManager;

    /**
     * @brief File writing component for performing actual file I/O operations
     *
     * Handles the low-level file writing operations, including opening files,
     * writing content, proper closing, and error handling. Different implementations
     * can provide various writing strategies (buffered, streaming, atomic, etc.).
     *
     * @see IFileWriter for file writing implementations and their characteristics
     */
    std::unique_ptr<IFileWriter> fileWriter;

public:

    /**
     * @brief Constructor that initializes DataExporter with injected dependencies
     *
     * Creates a DataExporter instance using dependency injection pattern. All required
     * components for data export operations are provided as constructor parameters,
     * allowing for flexible configuration and easy testing through dependency substitution.
     *
     * The injected dependencies handle different aspects of the export process:
     * - pathResolver: Determines the final export directory path
     * - directoryManager: Handles directory creation and validation
     * - fileWriter: Performs the actual file writing operations
     *
     * @param pathResolver Unique pointer to path resolution strategy implementation.
     *                     Takes ownership and will be used for all path resolution operations.
     * @param directoryManager Unique pointer to directory management implementation.
     *                         Takes ownership and will be used for directory operations.
     * @param fileWriter Unique pointer to file writing implementation.
     *                   Takes ownership and will be used for all file write operations.
     *
     * @note All parameters are moved into the DataExporter instance. The caller
     *       loses ownership of the passed unique_ptr objects.
     *
     * @see ExporterFactory::createExporter() for factory method that creates configured instances
     */
    DataExporter(
        std::unique_ptr<IPathResolver> pathResolver,
        std::unique_ptr<IDirectoryManager> directoryManager,
        std::unique_ptr<IFileWriter> fileWriter
    );

    /**
     * @brief Exports data to a file with automatic path resolution and directory creation
     *
     * Performs a complete data export operation by coordinating the three main components:
     * path resolution, directory management, and file writing. The export process follows
     * these steps:
     *
     * 1. **Path Resolution**: Uses the configured path resolver to determine the final
     *    export directory from the input path
     * 2. **Directory Creation**: Ensures the target directory exists, creating it if necessary
     * 3. **File Writing**: Writes the data to the specified filename in the resolved directory
     *
     * The method provides comprehensive error handling, catching all exceptions and
     * converting them to boolean return values for simplified error checking.
     *
     * @param path The base path for export. Interpretation depends on the configured
     *             path resolver (may be absolute, relative, or special tokens)
     * @param filename The name of the file to create in the resolved directory.
     *                 Should include file extension if desired.
     * @param data The string content to write to the file
     *
     * @return true if the export operation completed successfully (path resolved,
     *         directory created/verified, and file written), false otherwise
     *
     * @note Error details are logged to std::cerr when exceptions occur during export.
     *       For programmatic error handling, check the return value.
     *
     * @warning The method will overwrite existing files with the same name without warning.
     *          Ensure filename uniqueness or implement backup strategies if needed.
     *
     * @throws No exceptions are thrown - all errors are caught internally and result
     *         in false return values with error logging to stderr
     *
     * @see IPathResolver::resolvePath() for path resolution behavior
     * @see IDirectoryManager::ensureDirectoryExists() for directory creation details
     * @see IFileWriter::writeFile() for file writing implementation
     *
     * @example
     * ```cpp
     * DataExporter exporter(pathResolver, dirManager, fileWriter);
     *
     * if (exporter.exportData("output", "data.txt", "Hello World")) {
     *     std::cout << "Export successful" << std::endl;
     * } else {
     *     std::cout << "Export failed - check stderr for details" << std::endl;
     * }
     * ```
     */
    bool exportData(const std::string& path, const std::string& filename, const std::string& data) override;
};

