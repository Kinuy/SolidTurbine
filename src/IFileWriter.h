#pragma once

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief Interface for file writing operations
 *
 * IFileWriter provides an abstraction layer for writing string content to files,
 * enabling different file writing strategies and implementations through
 * polymorphism. Supports dependency injection for flexible output handling
 * and testing scenarios.
 *
 * ## Key Features
 * - **Strategy Pattern**: Multiple writing implementations (standard, atomic, buffered)
 * - **Path Flexibility**: Works with std::filesystem::path objects
 * - **Error Handling**: Boolean return values for simple success/failure checking
 * - **Testability**: Enables mock implementations for unit testing
 *
 * ## Common Implementations
 * - **StandardFileWriter**: Basic file I/O using std::ofstream
 * - **AtomicFileWriter**: Atomic write operations with temporary files
 * - **BufferedFileWriter**: High-performance buffered writing
 * - **MockFileWriter**: Testing implementation
 *
 * @see StandardFileWriter for standard implementation
 * @see DataExporter for typical usage in export workflows
 *
 * @example
 * ```cpp
 * std::unique_ptr<IFileWriter> writer =
 *     std::make_unique<StandardFileWriter>();
 *
 * bool success = writer->writeFile("output.txt", "Hello World");
 * ```
 */
class IFileWriter {

public:

    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~IFileWriter() = default;

    /**
     * @brief Writes string content to specified file path
     * @param filePath Target file path for writing
     * @param content String data to write to file
     * @return true if write operation completed successfully, false otherwise
     */
    virtual bool writeFile(const fs::path& filePath, const std::string& content) const = 0;

};