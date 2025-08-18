#pragma once

#include <any>
#include <string>
#include "IValueParser.h"

/**
 * @brief Pass-through parser implementation for string values
 *
 * StringParser implements IValueParser for string data types by returning input
 * values unchanged. This enables uniform handling of strings alongside other
 * data types in polymorphic parsing systems and configuration frameworks.
 *
 * ## Key Characteristics
 * - **No Conversion**: Returns input strings exactly as received
 * - **No Validation**: Accepts any std::string input without checking
 * - **Exception Safe**: Never throws exceptions
 * - **Thread Safe**: Safe for concurrent access
 *
 * ## Use Cases
 * - Configuration systems with mixed data types
 * - Factory-based parser systems requiring uniform interfaces
 * - Data processing pipelines handling heterogeneous input
 *
 * @see IValueParser for the base interface definition
 * @see DoubleParser, IntParser for converting parser examples
 *
 * @note While no validation is performed, downstream components can apply
 *       validation rules to parsed results as needed.
 *
 * @example
 * ```cpp
 * StringParser parser;
 * std::any result = parser.parse("Any text");
 * std::string value = std::any_cast<std::string>(result);
 *
 * // Polymorphic usage
 * std::unique_ptr<IValueParser> parser = std::make_unique<StringParser>();
 * std::any result = parser->parse("Configuration value");
 * ```
 */
class StringParser : public IValueParser {

public:

    /**
     * @brief Parses a string value by returning it unchanged
     *
     * This method implements the IValueParser interface for string values. Since no
     * type conversion is required for string-to-string parsing, the input value is
     * returned directly, wrapped in a std::any container to conform with the
     * interface contract.
     *
     * Unlike other parser implementations that perform validation and conversion
     * (e.g., DoubleParser, IntParser), StringParser accepts any input string
     * without modification or validation. This makes it suitable for:
     * - Configuration values that should be preserved as-is
     * - User input that requires no format restrictions
     * - Text data that will be processed by downstream components
     * - Default parsing when no specific type conversion is needed
     *
     * @param value The input string to parse. Any valid std::string is accepted,
     *              including empty strings, strings with whitespace, special
     *              characters, Unicode content, or any other string data.
     *
     * @return std::any containing the unchanged input string. The contained value
     *         can be extracted using std::any_cast<std::string>().
     *
     * @throws No exceptions are thrown by this method. All string inputs are
     *         considered valid and are accepted without validation.
     *
     * @note This parser performs no validation, sanitization, or format checking.
     *       If input validation is required, it should be performed by the caller
     *       or by a specialized string validator.
     *
     * @note The returned std::any always contains exactly the same string object
     *       that was passed as input, ensuring perfect fidelity of string data.
     *
     * @see IValueParser for the base interface definition
     * @see DoubleParser for an example of a converting parser
     * @see std::any_cast for extracting the string from the returned std::any
     *
     * @example
     * ```cpp
     * StringParser parser;
     *
     * // Basic string parsing
     * std::any result = parser.parse("Hello, World!");
     * std::string text = std::any_cast<std::string>(result); // "Hello, World!"
     *
     * // Empty strings are valid
     * std::any empty = parser.parse("");
     * std::string emptyStr = std::any_cast<std::string>(empty); // ""
     *
     * // Special characters and Unicode are preserved
     * std::any special = parser.parse("Café & 日本語 symbols: @#$%");
     * std::string preserved = std::any_cast<std::string>(special);
     * ```
     */
    std::any parse(const std::string& value) const override;

    /**
     * @brief Gets the type name identifier for this parser
     *
     * Returns a string that uniquely identifies this parser as handling string
     * values within the parsing system. This identifier is used for:
     * - Parser registration in factory classes and registries
     * - Type identification in configuration files and user interfaces
     * - Error messages and logging to identify which parser was used
     * - Runtime parser selection based on expected data types
     *
     * The returned identifier should remain constant across different versions
     * of the software to maintain compatibility with existing configuration
     * files, serialized data, and API contracts.
     *
     * @return The string "string" which serves as the canonical identifier
     *         for string parsing operations
     *
     * @note This identifier is case-sensitive and should be used exactly as
     *       returned when referencing this parser type in configuration or code.
     *
     * @note The identifier "string" follows common naming conventions for
     *       fundamental data types and aligns with programming language
     *       type naming standards.
     *
     * @see IValueParser::getTypeName() for the interface definition
     * @see ParserFactory for how type names are used in parser registration
     *
     * @example
     * ```cpp
     * StringParser parser;
     * std::string typeName = parser.getTypeName(); // Returns "string"
     *
     * // Usage in parser registration
     * ParserRegistry registry;
     * registry.registerParser(parser.getTypeName(), std::make_unique<StringParser>());
     *
     * // Configuration file example:
     * // parameter_name: "string" -> uses StringParser
     * // parameter_value: "any text value"
     * ```
     */
    std::string getTypeName() const override;

};

