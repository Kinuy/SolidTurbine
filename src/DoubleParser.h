#pragma once
#include <any>
#include <string>
#include <stdexcept>

#include "IValueParser.h"

/**
 * @brief Parser implementation for converting string values to double-precision floating-point numbers
 *
 * DoubleParser is a concrete implementation of the IValueParser interface that specializes
 * in parsing string representations of double-precision floating-point numbers. It provides
 * robust conversion capabilities with proper error handling and follows standard C++
 * floating-point parsing conventions.
 *
 * The parser supports a wide range of double formats including:
 * - Standard decimal notation (e.g., "3.14159", "-42.0")
 * - Scientific notation (e.g., "1.23e-4", "6.022E+23")
 * - Integer values that can be represented as doubles (e.g., "42", "-17")
 * - Special values like infinity and NaN (platform dependent)
 *
 * This parser is typically registered in a parser factory or registry system
 * and used for automatic type conversion in configuration files, command-line
 * arguments, or data processing pipelines where string-to-double conversion
 * is required.
 *
 * @see IValueParser for the base interface definition
 * @see std::stod for underlying conversion implementation details
 *
 * @note The parser performs strict validation and will throw exceptions for
 *       invalid input rather than returning default or truncated values
 *
 * @example
 * ```cpp
 * DoubleParser parser;
 * std::any result = parser.parse("3.14159");
 * double value = std::any_cast<double>(result);
 * ```
 */
class DoubleParser : public IValueParser {

public:

    /**
     * @brief Parses a string value and converts it to a double
     *
     * Attempts to convert the input string to a double-precision floating-point number
     * using std::stod. The parsed value is returned wrapped in a std::any container
     * to conform with the IValueParser interface.
     *
     * The parsing follows standard C++ floating-point string conversion rules:
     * - Accepts decimal notation (e.g., "3.14", "-2.5")
     * - Accepts scientific notation (e.g., "1.5e-3", "2.1E+4")
     * - Leading and trailing whitespace is ignored
     * - Supports positive and negative values
     *
     * @param value The string representation of the double value to parse
     * @return std::any containing the parsed double value
     * @throws std::invalid_argument if the string cannot be converted to a valid double,
     *         including cases where the string is empty, contains non-numeric characters,
     *         or represents a value outside the range of double precision
     *
     * @see std::stod for detailed parsing behavior and accepted formats
     */
    std::any parse(const std::string& value) const override;

    /**
     * @brief Gets the type name identifier for this parser
     *
     * Returns a string that identifies this parser as handling double-precision
     * floating-point values. This identifier is used by the parsing system
     * for type registration, error messages, and parser selection.
     *
     * @return The string "double" identifying this parser type
     */
    std::string getTypeName() const override;
};

