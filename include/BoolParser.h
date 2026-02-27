#pragma once

#include <algorithm>

#include "IValueParser.h"
#include <stdexcept>


/**
 * @brief Parser implementation for boolean value conversion
 *
 * BoolParser implements IValueParser to convert string representations
 * to boolean values. Provides case-insensitive parsing with support for
 * both textual and numeric boolean formats.
 *
 * ## Supported Formats
 * - **Textual**: "true"/"false" (case-insensitive)
 * - **Numeric**: "1"/"0" for true/false respectively
 * - **Flexible**: Handles mixed case input automatically
 *
 * ## Key Features
 * - Case-insensitive string matching
 * - Multiple representation support
 * - Clear error messages for invalid input
 * - Standard IValueParser interface compliance
 *
 * @see IValueParser for the base interface
 * @see StringParser, DoubleParser for other parser implementations
 *
 * @example
 * ```cpp
 * BoolParser parser;
 * bool flag = std::any_cast<bool>(parser.parse("TRUE"));  // true
 *
 * // Factory usage
 * auto parser = std::make_unique<BoolParser>();
 * std::any result = parser->parse("false");
 * ```
 */
class BoolParser : public IValueParser {

public:

    /**
     * @brief Parses string values to boolean with flexible format support
     *
     * Converts string representations to boolean values using case-insensitive
     * matching. Supports both textual ("true"/"false") and numeric ("1"/"0")
     * boolean representations.
     *
     * ## Accepted Values
     * - **True**: "true", "True", "TRUE", "1"
     * - **False**: "false", "False", "FALSE", "0"
     *
     * @param value String to parse as boolean
     * @return std::any containing boolean value
     * @throws std::invalid_argument if value doesn't match accepted formats
     *
     * @example
     * ```cpp
     * BoolParser parser;
     * bool result1 = std::any_cast<bool>(parser.parse("True"));  // true
     * bool result2 = std::any_cast<bool>(parser.parse("0"));     // false
     * ```
     */
    std::any parse(const std::string& value) const;

    /**
     * @brief Gets the type name identifier for this parser
     * @return The string "bool" identifying this parser type
     */
    std::string getTypeName() const;

};

