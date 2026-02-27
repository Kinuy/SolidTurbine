#pragma once

#include <string>

/**
 * @brief Interface for data export operations
 *
 * IExporter provides an abstraction layer for exporting string data to files,
 * enabling different export strategies and implementations through polymorphism.
 * Supports dependency injection for flexible output handling and testing.
 *
 * ## Key Features
 * - **Strategy Pattern**: Multiple export implementations (file, network, memory)
 * - **Path Flexibility**: Configurable output locations and filenames
 * - **Error Handling**: Boolean return values for simple success/failure checking
 * - **String-Based**: Handles text-based data export scenarios
 *
 * ## Common Implementations
 * - **DataExporter**: Standard file export with path resolution and directory management
 * - **NetworkExporter**: Remote storage export
 * - **MemoryExporter**: In-memory export for testing
 *
 * @see DataExporter for standard file-based implementation
 * @see ExporterFactory for factory-based exporter creation
 *
 * @example
 * ```cpp
 * std::unique_ptr<IExporter> exporter =
 *     ExporterFactory::createExporter(PathStrategy::FLEXIBLE, "/output");
 *
 * bool success = exporter->exportData("results", "data.txt", jsonContent);
 * ```
 */
class IExporter {

public:

    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~IExporter() = default;

    /**
     * @brief Exports string data to specified location
     * @param path Base path for export (interpretation depends on implementation)
     * @param filename Target filename for the exported data
     * @param data String content to export
     * @return true if export completed successfully, false otherwise
     */
    virtual bool exportData(const std::string& path, const std::string& filename, const std::string& data) = 0;

};