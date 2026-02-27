#pragma once
#include "ConfigurationParameter.h"
#include "DoubleParser.h"
#include "IntParser.h"
#include "StringParser.h"
#include "BoolParser.h"
#include "RangeParser.h"
#include "FilePathParser.h"

/**
 * @brief Configuration schema definition with parameter validation and type specification
 *
 * ConfigurationSchema defines the expected structure and validation rules for
 * configuration files through a collection of parameter definitions. Supports
 * both single-value and multi-value parameters with type-safe parsing and
 * validation. Uses fluent interface design for easy schema construction.
 *
 * ## Key Features
 * - **Type Safety**: Strongly typed parameter definitions with parser assignment
 * - **Validation Rules**: Required/optional parameter specification
 * - **Fluent Interface**: Method chaining for readable schema construction
 * - **Multi-Value Support**: Range parameters and complex parameter groups
 * - **File Handling**: Special support for data file and file list parameters
 *
 * ## Parameter Types
 * - **Primitive Types**: double, int, string, bool with built-in parsers
 * - **File Types**: File paths with validation and data loading
 * - **Range Types**: Start/end/step parameter groups
 * - **Custom Types**: User-defined parsers for complex data types
 *
 * ## Usage Pattern
 * 1. Create schema instance
 * 2. Add parameter definitions using fluent interface
 * 3. Pass to ConfigurationParser for file processing
 * 4. Parser validates input against schema rules
 *
 * @see ConfigurationParameter for individual parameter definitions
 * @see ConfigurationParser for schema-based parsing
 * @see IValueParser, IMultiValueParser for parser interfaces
 *
 * @example
 * ```cpp
 * ConfigurationSchema schema;
 * schema.addDouble("max_speed", true, "Maximum rotational speed [RPM]")
 *       .addString("project_name", false, "Project identifier")
 *       .addRange("angle_sweep", "start", "end", "step", true, "Angle range")
 *       .addDataFile("blade_geometry_file", true, "Blade geometry data")
 *       .addBool("enable_analysis", false, "Enable aerodynamic analysis");
 *
 * // Use schema with parser
 * ConfigurationParser parser(std::move(schema), std::move(fileReader));
 * Configuration config = parser.parse();
 * ```
 */
class ConfigurationSchema {

private:

    /**
     * @brief Collection of parameter definitions defining the configuration structure
     *
     * Each ConfigurationParameter contains the parameter name, associated parser,
     * requirement status, and documentation. The order of parameters in this
     * vector doesn't affect parsing but may influence validation error reporting.
     */
    std::vector<ConfigurationParameter> parameters;

public:

    /**
     * @brief Adds a single-value parameter to the configuration schema
     *
     * Registers a parameter that expects one value, using the provided parser
     * for type conversion. Supports fluent interface for method chaining.
     *
     * @param name Parameter name/identifier
     * @param parser Value parser for type conversion (ownership transferred)
     * @param required Whether parameter is mandatory (default: true)
     * @param description Optional parameter description
     * @return Reference to this schema for method chaining
     */
    ConfigurationSchema& add(
        const std::string& name, 
        std::unique_ptr<IValueParser> parser,
        bool required, 
        const std::string& description);

    /**
     * @brief Adds a multi-value parameter to the configuration schema
     *
     * Registers a parameter that expects multiple values, using the provided
     * multi-value parser for processing. Supports fluent interface.
     *
     * @param name Parameter name/identifier
     * @param parser Multi-value parser for processing (ownership transferred)
     * @param required Whether parameter is mandatory (default: true)
     * @param description Optional parameter description
     * @return Reference to this schema for method chaining
     */
    ConfigurationSchema& addMultiValue(
        const std::string& name, 
        std::unique_ptr<IMultiValueParser> parser,
        bool required, 
        const std::string& description);

    /**
     * @brief Gets all registered configuration parameters
     * @return Const reference to parameter vector
     */
    const std::vector<ConfigurationParameter>& getParameters() const;

    /**
     * @brief Convenience method to add a double-precision parameter
     * @param name Parameter name
     * @param required Whether mandatory (default: true)
     * @param desc Optional description
     * @return Reference to this schema for chaining
     */
    ConfigurationSchema& addDouble(
        const std::string& name, 
        bool required, 
        const std::string& description);

    /**
     * @brief Convenience method to add an integer parameter
     * @param name Parameter name
     * @param required Whether mandatory (default: true)
     * @param desc Optional description
     * @return Reference to this schema for chaining
     */
    ConfigurationSchema& addInt(
        const std::string& name, 
        bool required, 
        const std::string& desc);

    /**
     * @brief Convenience method to add a string parameter
     * @param name Parameter name
     * @param required Whether mandatory (default: true)
     * @param desc Optional description
     * @return Reference to this schema for chaining
     */
    ConfigurationSchema& addString(
        const std::string& name, 
        bool required, 
        const std::string& desc);

    /**
     * @brief Convenience method to add a boolean parameter
     * @param name Parameter name
     * @param required Whether mandatory (default: true)
     * @param desc Optional description
     * @return Reference to this schema for chaining
     */
    ConfigurationSchema& addBool(
        const std::string& name, 
        bool required, 
        const std::string& desc);

    /**
     * @brief Convenience method to add a data file path parameter
     * @param name Parameter name
     * @param required Whether mandatory (default: true)
     * @param desc Optional description
     * @return Reference to this schema for chaining
     */
    ConfigurationSchema& addDataFile(
        const std::string& name,
        bool required,
        const std::string& desc);

    /**
     * @brief Convenience method to add a double range parameter (start, end, step)
     * @param triggerKey Parameter name that triggers range parsing
     * @param startKey Output key for start value
     * @param endKey Output key for end value
     * @param stepKey Output key for step value
     * @param required Whether mandatory (default: true)
     * @param desc Optional description
     * @return Reference to this schema for chaining
     */
    ConfigurationSchema& addRange(
        const std::string& triggerKey, 
        const std::string& startKey,
        const std::string& endKey, 
        const std::string& stepKey,
        bool required, 
        const std::string& desc);

    /**
    * @brief Convenience method to add an integer range parameter (start, end, step)
    * @param triggerKey Parameter name that triggers range parsing
    * @param startKey Output key for start value
    * @param endKey Output key for end value
    * @param stepKey Output key for step value
    * @param required Whether mandatory (default: true)
    * @param desc Optional description
    * @return Reference to this schema for chaining
    */
    ConfigurationSchema& addIntRange(
        const std::string& triggerKey, 
        const std::string& startKey,
        const std::string& endKey, 
        const std::string& stepKey,
        bool required, 
        const std::string& desc);

    /**
     * @brief Checks if a parameter is configured as a data file parameter
     * @param name Parameter name to check
     * @return true if parameter exists and uses FilePathParser
     */
    bool isDataFileParameter(const std::string& name) const;

};

