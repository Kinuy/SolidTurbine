#pragma once


#include <memory>
#include <string>

#include "PathStrategy.h"
#include "IExporter.h"
#include "IPathResolver.h"
#include "DefaultPathResolver.h"
#include "FlexiblePathResolver.h"
#include "DataExporter.h"
#include "FileSystemDirectoryManager.h"
#include "StandardFileWriter.h"

/**
 * @brief Factory class for creating configured data exporter instances
 *
 * ExporterFactory provides factory methods to create fully configured data
 * exporters with appropriate path resolution strategies, directory managers,
 * and file writers. Encapsulates the complexity of assembling exporter
 * dependencies through a simple interface.
 *
 * ## Key Features
 * - **Strategy Selection**: Creates exporters with specified path strategies
 * - **Dependency Assembly**: Automatically wires required components
 * - **Configuration Abstraction**: Hides complex exporter setup details
 * - **Consistent Creation**: Ensures properly configured exporter instances
 *
 * ## Available Strategies
 * - **PathStrategy::DEFAULT**: Conservative path handling with default directory
 * - **PathStrategy::FLEXIBLE**: Flexible path handling preserving user structure
 *
 * @see IExporter for exporter interface
 * @see DataExporter for concrete exporter implementation
 * @see PathStrategy for available path resolution strategies
 *
 * @example
 * ```cpp
 * auto exporter = ExporterFactory::createExporter(
 *     PathStrategy::FLEXIBLE, "/output/data");
 * bool success = exporter->exportData("results", "data.txt", content);
 * ```
 */
class ExporterFactory {

public:

    /**
     * @brief Factory method that creates a configured data exporter instance
     *
     * Creates and assembles all necessary components for data exporting, including
     * a path resolver (based on the specified strategy), directory manager, and file writer.
     * The factory encapsulates the complexity of creating and wiring these dependencies.
     *
     * Available path strategies:
     * - DEFAULT: Uses DefaultPathResolver for simple, predictable path handling
     * - FLEXIBLE: Uses FlexiblePathResolver for more adaptable path resolution
     *
     * The created exporter is fully configured and ready to use for data export operations.
     *
     * @param strategy The path resolution strategy to use for determining output file locations
     * @param defaultDirectory The base directory to use for file output operations
     * @return A unique pointer to a fully configured IExporter instance
     * @throws std::bad_alloc if memory allocation fails during component creation
     * @throws std::invalid_argument if defaultDirectory is invalid or inaccessible (depending on path resolver implementation)
     *
     * @see PathStrategy for available strategy options
     * @see IExporter for the interface definition
     * @see DataExporter for the concrete implementation details
     */
    static std::unique_ptr<IExporter> createExporter(
        PathStrategy strategy = PathStrategy::DEFAULT,
        const std::string& defaultDirectory = "export"
    );
};