#pragma once
#include <vector>
#include <string>


/**
 * @brief Interface for file reading operations
 *
 * IFileReader provides an abstraction layer for reading text files,
 * enabling different file access strategies and implementations through
 * polymorphism. Supports dependency injection for flexible input handling.
 *
 * ## Key Features
 * - **Strategy Pattern**: Multiple reading implementations (file, network, memory)
 * - **Line-Based Access**: Returns content as vector of strings for easy processing
 * - **Testability**: Enables mock implementations for unit testing
 * - **Platform Abstraction**: Isolates file I/O behind interface
 *
 * ## Common Implementations
 * - **FileReader**: Standard filesystem file reading
 * - **NetworkFileReader**: Remote file access
 * - **MemoryFileReader**: In-memory content for testing
 *
 * @see FileReader for standard implementation
 * @see ConfigurationParser for typical usage context
 *
 * @example
 * ```cpp
 * std::unique_ptr<IFileReader> reader =
 *     std::make_unique<FileReader>("config.txt");
 *
 * auto lines = reader->readLines();
 * ```
 */
class IFileReader {

public:

    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~IFileReader() = default;

    /**
     * @brief Reads all lines from the input source
     * @return Vector of strings, each representing one line from the source
     * @throws std::runtime_error if reading fails or source is inaccessible
     */
    virtual std::vector<std::string> readLines() = 0;

};
