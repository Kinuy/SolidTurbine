#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>

#include "IFileReader.h"

/**
 * @brief Simple file reader implementation for text-based configuration files
 *
 * FileReader provides basic file reading capabilities by implementing the
 * IFileReader interface. Reads entire files line-by-line into memory for
 * processing by configuration parsers.
 *
 * ## Key Features
 * - **Simple Interface**: Basic file-to-memory reading
 * - **Line Preservation**: Maintains original line structure
 * - **Error Handling**: Clear exceptions for file access issues
 * - **Memory Loading**: Loads entire file for random access
 *
 * ## Use Cases
 * - Configuration file reading
 * - Small to medium text file processing
 * - Sequential line-based parsing workflows
 *
 * @see IFileReader for the base interface
 * @see ConfigurationParser for typical usage context
 *
 * @note Loads entire file into memory - not suitable for very large files
 * @note No encoding detection - assumes system default encoding
 *
 * @example
 * ```cpp
 * auto reader = std::make_unique<FileReader>("config.txt");
 * auto lines = reader->readLines();
 * ```
 */
class FileReader : public IFileReader {

private:

    /**
     * @brief Path to the target file for reading operations
     */
    std::string filename;

public:

    /**
     * @brief Constructor initializing file reader with target filename
     * @param filename Path to file for reading operations
     */
    explicit FileReader(const std::string& filename);

    /**
     * @brief Reads all lines from the file into a string vector
     * @return Vector containing all file lines as strings
     * @throws std::runtime_error if file cannot be opened
     * @note Preserves original line content including whitespace
     */
    std::vector<std::string> readLines() override;
};

