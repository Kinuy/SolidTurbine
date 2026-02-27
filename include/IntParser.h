#pragma once

#include "IValueParser.h"
#include <any>
#include <string>
#include <stdexcept>


/**
 * @brief Parser implementation for integer value conversion
 *
 * IntParser implements IValueParser to convert string representations
 * to integer values using std::stoi. Provides robust error handling
 * with clear error messages for invalid input.
 *
 * ## Key Features
 * - **Standard Conversion**: Uses std::stoi for reliable parsing
 * - **Error Handling**: Clear exceptions for invalid input
 * - **Range Support**: Handles full int range supported by std::stoi
 * - **Whitespace Handling**: Automatically trims leading whitespace
 *
 * ## Supported Formats
 * - Decimal integers: "42", "-17", "+123"
 * - Leading whitespace is ignored
 * - Trailing non-digit characters cause parsing errors
 *
 * @see IValueParser for the base interface
 * @see DoubleParser, StringParser for other parser implementations
 *
 * @example
 * ```cpp
 * IntParser parser;
 * int count = std::any_cast<int>(parser.parse("100"));
 *
 * // Factory usage
 * auto parser = std::make_unique<IntParser>();
 * ```
 */
class IntParser : public IValueParser {

public:

    /**
     * @brief Parses string values to integer with error handling
     *
     * Converts string representations to integer values using std::stoi.
     * Provides clear error messages for invalid input formats.
     *
     * @param value String to parse as integer
     * @return std::any containing integer value
     * @throws std::invalid_argument if string cannot be converted to valid integer
     *
     * @example
     * ```cpp
     * IntParser parser;
     * int result = std::any_cast<int>(parser.parse("42"));  // 42
     * ```
     */
    std::any parse(const std::string& value) const override;

    /**
     * @brief Gets the type name identifier for this parser
     * @return The string "int" identifying this parser type
     */
    std::string getTypeName() const override;
};

