#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

/**
 * @brief Static utility class for parsing configuration file lines
 *
 * LineParser provides functionality to parse individual lines from configuration
 * files into structured data. Handles various line types including empty lines,
 * comments, single-value parameters, and multi-value parameters with robust
 * error handling and whitespace management.
 *
 * ## Key Features
 * - **Comment Detection**: Recognizes comment lines starting with '#' or ';'
 * - **Inline Comment Removal**: Strips comments from end of parameter lines
 * - **Multi-Value Support**: Handles parameters with multiple values
 * - **Whitespace Handling**: Automatic trimming and normalization
 * - **Static Interface**: Pure utility class, no instantiation required
 *
 * ## Supported Line Formats
 * - Empty lines and whitespace-only lines
 * - Comment lines: `# This is a comment` or `; Alternative comment`
 * - Single value: `parameter_name value`
 * - Multi-value: `parameter_name value1 value2 value3`
 * - Inline comments: `parameter_name value # end-of-line comment`
 *
 * @see ParsedLine for the structure containing parsed line data
 * @see ConfigurationParser for typical usage context
 *
 * @example
 * ```cpp
 * auto parsed = LineParser::parseLine("max_speed 25.0 # RPM limit");
 * if (!parsed.isEmpty && !parsed.isComment) {
 *     std::string key = parsed.key;           // "max_speed"
 *     std::string value = parsed.value;       // "25.0"
 * }
 * ```
 */
class LineParser {

private:

    /**
     * @brief Trims whitespace and removes inline comments from string
     *
     * Removes everything after '#' character (inline comments) and trims
     * leading/trailing whitespace from the remaining content.
     *
     * @param str Input string to clean
     * @return Cleaned string with comments and whitespace removed
     *
     * @example
     * ```cpp
     * std::string clean = LineParser::trim("  key value  # comment  ");
     * // Returns: "key value"
     * ```
     */
    static std::string trim(const std::string& str);

public:

    /**
     * @brief Structure representing a parsed configuration file line
     *
     * ParsedLine contains the extracted components and metadata from parsing
     * a single configuration file line. Supports both single and multi-value
     * parameters with appropriate flagging for different line types.
     *
     * ## Usage Pattern
     * The structure provides both legacy single-value access and modern
     * multi-value support through separate fields, enabling backward
     * compatibility while supporting complex parameter formats.
     *
     * @see LineParser::parseLine() for creation of ParsedLine instances
     *
     * @example
     * ```cpp
     * ParsedLine parsed = LineParser::parseLine("range_param 0.0 10.0 0.5");
     * // parsed.key = "range_param"
     * // parsed.value = "0.0" (first value for compatibility)
     * // parsed.values = {"0.0", "10.0", "0.5"}
     * // parsed.hasMultipleValues = true
     * ```
     */
    struct ParsedLine {

        /**
         * @brief Parameter name/identifier extracted from line
         */
        std::string key;

        /**
         * @brief First value for backward compatibility with single-value parsers
         */
        std::string value;  // For single values

        /**
         * @brief All values from the line for multi-value parameter support
         */
        std::vector<std::string> values;  // For multiple values

        /**
         * @brief True if line contains only whitespace
         */
        bool isEmpty;

        /**
         * @brief True if line is a comment (starts with # or ;)
         */
        bool isComment;

        /**
         * @brief True if line contains more than one value
         */
        bool hasMultipleValues;

    };

    /**
     * @brief Parses a configuration line into structured components
     *
     * Processes a configuration file line by extracting the parameter key,
     * values, and metadata. Handles empty lines, comments, single values,
     * and multi-value parameters with appropriate flagging.
     *
     * @param line Raw configuration line to parse
     * @return ParsedLine structure containing key, values, and parsing metadata
     * @throws std::invalid_argument if line format is invalid or no values found
     *
     * @note Comment lines starting with '#' or ';' are detected but not parsed
     * @note Inline comments (# within line) are automatically removed
     *
     * @example
     * ```cpp
     * auto parsed = LineParser::parseLine("max_speed 25.0 30.0");
     * // parsed.key = "max_speed", parsed.values = {"25.0", "30.0"}
     * ```
     */
    static ParsedLine parseLine(const std::string& line);


};
