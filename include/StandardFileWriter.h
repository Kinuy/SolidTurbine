#pragma once

#include <filesystem>
#include <string>
#include <iostream>
#include <stdexcept>
#include <fstream>

#include "IFileWriter.h"

namespace fs = std::filesystem;

/**
 * @brief Standard file writing implementation using std::ofstream
 *
 * StandardFileWriter implements IFileWriter using standard C++ file I/O
 * operations. Provides straightforward file writing with automatic error
 * handling, console logging, and support for filesystem paths.
 *
 * ## Key Features
 * - **Standard I/O**: Uses std::ofstream for reliable file operations
 * - **Error Handling**: Comprehensive exception catching and boolean returns
 * - **Console Logging**: Success messages to stdout, errors to stderr
 * - **Path Support**: Works with std::filesystem::path objects
 *
 * ## Behavior
 * - Creates new files or overwrites existing ones without warning
 * - Automatically closes files after writing
 * - Returns boolean status for easy error checking
 *
 * @see IFileWriter for the base interface definition
 * @see FileSystemDirectoryManager for directory creation companion
 *
 * @note This implementation prioritizes simplicity and reliability over
 *       advanced features like atomic writes or backup creation.
 *
 * @example
 * ```cpp
 * StandardFileWriter writer;
 * bool success = writer.writeFile("data.txt", "content");
 *
 * // Dependency injection usage
 * std::unique_ptr<IFileWriter> writer = std::make_unique<StandardFileWriter>();
 * ```
 */
class StandardFileWriter : public IFileWriter {

public:

    /**
     * @brief Writes string content to a file at the specified path
     *
     * Creates or overwrites a file with the provided content using standard
     * file I/O operations. Provides comprehensive error handling and logging
     * for both successful writes and failures.
     *
     * ## Behavior
     * - **File Creation**: Creates new files or overwrites existing ones
     * - **Error Handling**: Catches exceptions and returns boolean status
     * - **Logging**: Outputs success/failure messages to console streams
     * - **Path Support**: Accepts any valid filesystem path
     *
     * @param filePath Target file path where content will be written
     * @param content String data to write to the file
     * @return true if file was written successfully, false otherwise
     *
     * @note Overwrites existing files without warning. Ensure file backup
     *       or uniqueness checks are performed by caller if needed.
     *
     * @warning Success messages go to stdout, errors to stderr. Ensure
     *          appropriate stream handling in applications requiring silence.
     *
     * @example
     * ```cpp
     * StandardFileWriter writer;
     * bool success = writer.writeFile("output.txt", "Hello World");
     * if (!success) {
     *     // Handle write failure
     * }
     * ```
     */
    bool writeFile(const fs::path& filePath, const std::string& content) const override;
};
