#pragma once

#include<memory>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <stdexcept>
#include <any>

#include "IValueParser.h"
#include "IDataFileParser.h"
#include "IFileListParser.h"
#include "IStructuredData.h"

#include "BladeGeometryParser.h"
#include "AirfoilPerformanceParser.h"
#include "AirfoilPerformanceFileListParser.h"
#include "AirfoilGeometryFileListParser.h"
#include "AirfoilGeometryParser.h"


/**
 * @brief File path parser with delegated parsing capabilities
 *
 * FilePathParser implements IValueParser while serving as a factory for
 * different types of file parsers. Maintains registries of data file parsers
 * and file list parsers, determining the appropriate parser based on
 * parameter names and file extensions.
 *
 * ## Key Features
 * - **Parser Registry**: Maintains collections of specialized file parsers
 * - **Type Detection**: Automatic parser selection based on parameter naming
 * - **File Validation**: Ensures file existence during parsing
 * - **Flexible Delegation**: Supports both individual files and file lists
 *
 * @see IValueParser for the base interface
 * @see IDataFileParser, IFileListParser for delegated parser types
 *
 * @example
 * ```cpp
 * FilePathParser parser;
 * auto data = parser.parseDataFile("blade_geometry_file", "blade.dat");
 * ```
 */
class FilePathParser : public IValueParser {

private:

    /**
     * @brief Registry of data file parsers indexed by file type
     *
     * Maps file type strings (e.g., "blade_geometry", "airfoil_performance")
     * to their corresponding parser implementations for handling individual
     * data files.
     */
    std::unordered_map<std::string, std::unique_ptr<IDataFileParser>> dataFileParsers;

    /**
     * @brief Registry of file list parsers indexed by list type
     *
     * Maps file list type strings (e.g., "airfoil_performance_files") to
     * their corresponding parser implementations for handling files that
     * contain lists of other files.
     */
    std::unordered_map<std::string, std::unique_ptr<IFileListParser>> fileListParsers;

    /**
     * @brief Extracts file type from parameter name for data file parsing
     * @param parameterName Parameter name to analyze
     * @return File type string for parser selection
     */
    std::string extractFileType(const std::string& parameterName) const;

    /**
     * @brief Extracts file list type from parameter name for list parsing
     * @param parameterName Parameter name to analyze
     * @return File list type string for parser selection
     */
    std::string extractFileListType(const std::string& parameterName) const;

public:

    /**
     * @brief Constructor that initializes the FilePathParser with supported parsers
     *
     * Registers all available data file parsers and file list parsers in their
     * respective registries. Data file parsers handle individual data files,
     * while file list parsers handle files containing lists of other files.
     *
     * Currently supported data file types:
     * - blade_geometry: Parses blade geometry data files
     * - airfoil_geometry: Parses airfoil geometry data files
     * - airfoil_performance: Parses airfoil performance data files
     *
     * Currently supported file list types:
     * - airfoil_performance_files: Parses lists of airfoil performance files
     * - airfoil_geometry_files: Parses lists of airfoil geometry files
     */
    FilePathParser();

    /**
     * @brief Parses and validates a file path string
     *
     * Validates that the specified file path exists in the filesystem.
     * This method implements the IValueParser interface requirement.
     *
     * @param value The file path string to validate and parse
     * @return std::any containing the validated file path as a string
     * @throws std::invalid_argument if the file does not exist at the specified path
     */
    std::any parse(const std::string& value) const;

    /**
     * @brief Gets the type name identifier for this parser
     *
     * Returns a string that identifies this parser type for use in
     * type registration and identification systems.
     *
     * @return The string "filepath" identifying this parser type
     */
    std::string getTypeName() const;

    /**
     * @brief Check if a parameter represents a file list type
     *
     * Determines whether the given parameter name indicates a file list
     * parameter by checking for the "_files_file" pattern in the name.
     *
     * @param parameterName The parameter name to check
     * @return true if the parameter represents a file list, false otherwise
     */
    bool isFileListParameter(const std::string& parameterName) const;

    /**
     * @brief Parse a data file based on parameter name
     *
     * Extracts the file type from the parameter name, finds the appropriate
     * data file parser, and uses it to parse the specified file.
     *
     * @param parameterName The parameter name used to determine file type (e.g., "blade_geometry_file")
     * @param filePath The path to the data file to parse
     * @return A unique pointer to the parsed structured data
     * @throws std::runtime_error if no parser is available for the extracted file type
     * @throws std::invalid_argument if the file cannot be read or parsed
     */
    std::unique_ptr<IStructuredData> parseDataFile(const std::string& parameterName,
        const std::string& filePath) const;

    /**
     * @brief Parse a file list file based on parameter name
     *
     * Extracts the file list type from the parameter name, finds the appropriate
     * file list parser, and uses it to parse the specified file containing a list of other files.
     *
     * @param parameterName The parameter name used to determine file list type (e.g., "airfoil_performance_files_file")
     * @param filePath The path to the file list file to parse
     * @return A unique pointer to the parsed structured data representing the file list
     * @throws std::runtime_error if no file list parser is available for the extracted type
     * @throws std::invalid_argument if the file cannot be read or parsed
     */
    std::unique_ptr<IStructuredData> parseFileListFile(const std::string& parameterName,
        const std::string& filePath) const;

    /**
     * @brief Parse an individual file with explicit file type specification
     *
     * Directly parses a single file using the specified file type, bypassing
     * parameter name-based type detection. This is useful for parsing individual
     * files from a file list where the type is already known.
     *
     * @param filePath The path to the individual file to parse
     * @param fileType The explicit file type to use for parser selection
     * @return A unique pointer to the parsed structured data
     * @throws std::runtime_error if no parser is available for the specified file type
     * @throws std::invalid_argument if the file cannot be read or parsed
     */
    std::unique_ptr<IStructuredData> parseIndividualFile(const std::string& filePath,
        const std::string& fileType) const;

};

