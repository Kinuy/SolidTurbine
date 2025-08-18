#pragma once
#include "ConfigurationSchema.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <vector>
#include <filesystem>
#include <iostream>

#include "IValueParser.h"
#include "IMultiValueParser.h"
#include "IFileReader.h"

#include "Configuration.h"
#include "LineParser.h"


/**
 * @brief Configuration file parser with schema-driven validation and data loading
 *
 * ConfigurationParser processes configuration files according to a predefined
 * schema, supporting both simple values and complex data structures. Handles
 * automatic data file loading, multi-value parameters, and comprehensive
 * validation with detailed error reporting.
 *
 * ## Key Features
 * - **Schema-Driven**: Uses ConfigurationSchema for parameter definitions
 * - **Mixed Parameter Types**: Single values, multi-values, and file references
 * - **Automatic File Loading**: Loads referenced data files and file lists
 * - **Validation**: Required parameter checking with line-level error reporting
 * - **Flexible Input**: Pluggable file readers for different sources
 *
 * ## Processing Flow
 * 1. Parse configuration lines according to schema
 * 2. Load referenced data files and collections
 * 3. Validate required parameters are present
 * 4. Return populated Configuration object
 *
 * @see ConfigurationSchema for parameter definitions
 * @see Configuration for parsed data storage
 * @see IFileReader for input source abstraction
 *
 * @example
 * ```cpp
 * ConfigurationSchema schema;
 * schema.addDouble("speed").addDataFile("blade_geometry_file");
 *
 * ConfigurationParser parser(std::move(schema),
 *                           std::make_unique<TextFileReader>("config.txt"));
 * Configuration config = parser.parse();
 * ```
 */
class ConfigurationParser {

private:

    /**
     * @brief Configuration schema defining expected parameters and their types
     */
    ConfigurationSchema schema;

    /**
     * @brief File reader for accessing configuration input source
     */
    std::unique_ptr<IFileReader> fileReader;

    /**
     * @brief Organized parser lookup tables for efficient processing
     *
     * ParserMaps provides categorized access to different parser types and
     * metadata extracted from the schema. Used internally to optimize
     * parsing performance and organize validation requirements.
     */
    struct ParserMaps {

        /**
         * @brief Map of single-value parser references by parameter name
         */
        std::unordered_map<std::string, const IValueParser*> singleValueParsers;

        /**
         * @brief Map of multi-value parser references by parameter name
         */
        std::unordered_map<std::string, const IMultiValueParser*> multiValueParsers;

        /**
         * @brief Map of parameter requirement flags by key name
         */
        std::unordered_map<std::string, bool> requiredKeys;

        /**
         * @brief List of parameters that reference individual data files
         */
        std::vector<std::string> dataFileParams; 

        /**
         * @brief List of parameters that reference file lists
         */
        std::vector<std::string> fileListParams; 

    };

    /**
     * @brief Creates parser lookup maps from schema for efficient processing
     * @return ParserMaps containing organized parser references and metadata
     */
    ParserMaps createParserMaps() const;

    /**
     * @brief Validates that all required parameters are present in configuration
     * @param config Configuration to validate
     * @param parserMaps Parser maps containing requirement information
     * @throws std::runtime_error if required parameters are missing
     */
    void validateRequiredParameters(const Configuration& config, const ParserMaps& parserMaps) const;

    /**
     * @brief Loads all data files (individual files and file lists) specified in configuration
     * @param config Configuration to populate with structured data
     * @param parserMaps Parser maps containing file parameter information
     * @throws std::runtime_error if critical data files cannot be loaded
     */
    void loadDataFiles(Configuration& config, const ParserMaps& parserMaps) const;

    /**
     * @brief Extracts data key from parameter name by removing "_file" suffix
     * @param paramName Parameter name (e.g., "blade_geometry_file")
     * @return Data key (e.g., "blade_geometry")
     */
    std::string extractDataKey(const std::string& paramName) const;

    /**
     * @brief Extracts file list data key from parameter name
     * @param paramName Parameter name (e.g., "airfoil_performance_files_file")
     * @return File list data key (e.g., "airfoil_performance_files")
     */
    std::string extractFileListDataKey(const std::string& paramName) const;

    /**
     * @brief Loads individual airfoil geometry files from file list
     * @param config Configuration to populate with loaded geometries
     * @param filePathParser Parser for handling individual files
     * @param fileListKey Key for accessing the geometry file list
     * @note Continues processing even if individual files fail to load
     */
    void loadIndividualAirfoilGeometries(Configuration& config, const FilePathParser* filePathParser,
        const std::string& fileListKey) const;

    /**
     * @brief Loads individual airfoil performance files from file list
     * @param config Configuration to populate with loaded performance data
     * @param filePathParser Parser for handling individual files
     * @param fileListKey Key for accessing the performance file list
     * @note Continues processing even if individual files fail to load
     */
    void loadIndividualAirfoilPerformances(Configuration& config, const FilePathParser* filePathParser,
        const std::string& fileListKey) const;


public:

    /**
     * @brief Constructor initializing parser with schema and file reader
     * @param schema Configuration schema defining expected parameters
     * @param reader File reader implementation for input source
     */
    ConfigurationParser(ConfigurationSchema schema, std::unique_ptr<IFileReader> reader);

    /**
     * @brief Parses configuration file and returns populated Configuration object
     * @return Complete Configuration with parsed values and loaded data files
     * @throws std::runtime_error for parsing errors or missing required parameters
     */
    Configuration parse();

};

