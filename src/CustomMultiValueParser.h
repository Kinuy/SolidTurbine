#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

#include "IValueParser.h"
#include "IMultiValueParser.h"



/**
 * @brief Configurable multi-value parser that maps positional string values to typed results
 *
 * CustomMultiValueParser is a flexible implementation of IMultiValueParser that allows
 * dynamic configuration of parsing rules through key-parser registration. This class
 * enables parsing of multiple related string values in a single operation, with each
 * value being processed by its corresponding specialized parser and the results organized
 * in a key-value map.
 *
 * ## Architecture
 *
 * The parser maintains an ordered collection of key-parser pairs, where:
 * - **Keys**: String identifiers for the parsed values in the result map
 * - **Parsers**: Specialized IValueParser implementations for converting strings to typed values
 * - **Order**: Registration order determines the expected order of input values
 *
 * ## Key Features
 *
 * - **Dynamic Configuration**: Parsers can be added at runtime using fluent interface
 * - **Type Safety**: Each value is parsed by its dedicated type-specific parser
 * - **Positional Mapping**: Input values are matched to parsers by position, not by name
 * - **Strict Validation**: Enforces exact count matching between input values and registered parsers
 * - **Flexible Output**: Results are returned as a map for easy access by key name
 *
 * ## Usage Pattern
 *
 * 1. **Configuration Phase**: Register key-parser pairs using addValue() with method chaining
 * 2. **Parsing Phase**: Call parseMultiple() with a vector of string values
 * 3. **Result Access**: Extract typed values from the returned map using std::any_cast
 *
 * ## Design Benefits
 *
 * - **Reusability**: Same parser configuration can process multiple value sets
 * - **Maintainability**: Clear separation between parser configuration and data processing
 * - **Extensibility**: New value types can be added by implementing IValueParser
 * - **Error Isolation**: Individual parser failures don't affect parser configuration
 *
 * @see IMultiValueParser for the base interface definition
 * @see IValueParser for individual value parser implementations
 *
 * @note This class is not thread-safe during configuration (addValue calls).
 *       Once configured, parsing operations (parseMultiple) are read-only and thread-safe.
 *
 * @warning The order of addValue() calls is critical - it determines the expected
 *          order of values in parseMultiple(). Changing registration order changes
 *          the parsing behavior.
 *
 * @example
 * ```cpp
 * // Configure parser for geometry data: width, height, depth, name
 * CustomMultiValueParser geometryParser;
 * geometryParser.addValue("width", std::make_unique<DoubleParser>())
 *               .addValue("height", std::make_unique<DoubleParser>())
 *               .addValue("depth", std::make_unique<DoubleParser>())
 *               .addValue("name", std::make_unique<StringParser>());
 *
 * // Parse multiple geometry entries
 * std::vector<std::string> values1 = {"10.5", "20.3", "5.0", "box1"};
 * std::vector<std::string> values2 = {"15.2", "8.7", "12.1", "box2"};
 *
 * auto result1 = geometryParser.parseMultiple(values1);
 * auto result2 = geometryParser.parseMultiple(values2);
 *
 * // Extract typed values
 * double width1 = std::any_cast<double>(result1["width"]);   // 10.5
 * std::string name2 = std::any_cast<std::string>(result2["name"]); // "box2"
 * ```
 *
 * @example
 * ```cpp
 * // Error handling example
 * try {
 *     // Wrong number of values
 *     std::vector<std::string> wrongCount = {"10.5", "20.3"}; // Missing 2 values
 *     auto result = geometryParser.parseMultiple(wrongCount);
 * } catch (const std::invalid_argument& e) {
 *     std::cout << "Count mismatch: " << e.what() << std::endl;
 * }
 *
 * try {
 *     // Invalid value format
 *     std::vector<std::string> invalidData = {"not_a_number", "20.3", "5.0", "box"};
 *     auto result = geometryParser.parseMultiple(invalidData);
 * } catch (const std::invalid_argument& e) {
 *     std::cout << "Parse error: " << e.what() << std::endl;
 * }
 * ```
 */
class CustomMultiValueParser : public IMultiValueParser {

private:

    /**
     * @brief Ordered collection of key-parser pairs that define the parsing configuration
     *
     * This vector maintains the parsing specification as pairs of:
     * - **Key (string)**: The identifier used in the result map for the parsed value
     * - **Parser (unique_ptr<IValueParser>)**: The specialized parser for converting
     *   the string value to its target type
     *
     * ## Order Significance
     * The order of elements in this vector determines the expected order of input
     * values in parseMultiple(). The first pair corresponds to values[0], the
     * second pair to values[1], and so on.
     *
     * ## Ownership Semantics
     * The vector holds unique_ptr objects, meaning this class owns all registered
     * parsers and manages their lifetime. Parsers are moved into the vector during
     * addValue() calls and are automatically cleaned up when the CustomMultiValueParser
     * is destroyed.
     *
     * ## Access Pattern
     * - **Registration**: New pairs are added via addValue() using emplace_back()
     * - **Parsing**: Pairs are accessed by index during parseMultiple() operations
     * - **Introspection**: Pairs are iterated for getOutputKeys() and getExpectedValueCount()
     *
     * @see addValue() for registration mechanism
     * @see parseMultiple() for index-based access during parsing
     */
    std::vector<std::pair<std::string, std::unique_ptr<IValueParser>>> keyParsers;

public:

    /**
     * @brief Adds a key-parser pair to the multi-value parser configuration
     *
     * Registers a new key-parser combination that will be used during multi-value parsing.
     * The order of registration determines the expected order of values in parseMultiple().
     * This method supports method chaining through its fluent interface design.
     *
     * The parser takes ownership of the provided IValueParser instance and will use it
     * to parse the corresponding positional value during parseMultiple() operations.
     *
     * @param key The key name that will be associated with the parsed value in the result map.
     *            Must be unique within this parser instance to avoid key collisions.
     * @param parser Unique pointer to the value parser that will handle the conversion
     *               for this key. Ownership is transferred to the CustomMultiValueParser.
     *
     * @return Reference to this CustomMultiValueParser instance for method chaining
     *
     * @note The order of addValue() calls determines the expected order of values
     *       in the parseMultiple() method. First registered key corresponds to
     *       values[0], second to values[1], etc.
     *
     * @warning If the same key is added multiple times, the result map will contain
     *          only the value from the last occurrence, potentially causing data loss.
     *
     * @see parseMultiple() for usage of registered key-parser pairs
     *
     * @example
     * ```cpp
     * CustomMultiValueParser parser;
     * parser.addValue("width", std::make_unique<DoubleParser>())
     *       .addValue("height", std::make_unique<DoubleParser>())
     *       .addValue("name", std::make_unique<StringParser>());
     * ```
     */
    CustomMultiValueParser& addValue(const std::string& key, std::unique_ptr<IValueParser> parser);

    /**
     * @brief Parses multiple string values using registered key-parser pairs
     *
     * Processes a vector of string values by applying the corresponding registered parsers
     * in the order they were added via addValue(). Each value is parsed using its
     * associated parser and stored in the result map with its corresponding key.
     *
     * The method enforces strict value count matching - the number of input values
     * must exactly match the number of registered key-parser pairs. This ensures
     * predictable parsing behavior and prevents silent data loss or misalignment.
     *
     * @param values Vector of string values to parse. Must contain exactly the same
     *               number of elements as registered key-parser pairs. The order
     *               must match the registration order from addValue() calls.
     *
     * @return Unordered map containing parsed values keyed by their registered names.
     *         Each value is wrapped in std::any and can be extracted using std::any_cast.
     *
     * @throws std::invalid_argument if the number of input values doesn't match
     *         the number of registered key-parser pairs
     * @throws Various parser-specific exceptions (e.g., std::invalid_argument)
     *         if individual value parsing fails. The specific exception depends
     *         on the parser implementation for the failing value.
     *
     * @note Parsing stops at the first error. If parsing fails for any value,
     *       no partial results are returned.
     *
     * @see addValue() for registering key-parser pairs
     * @see getExpectedValueCount() for determining required value count
     *
     * @example
     * ```cpp
     * // Assuming parser configured with width(double), height(double), name(string)
     * std::vector<std::string> values = {"3.14", "2.71", "rectangle"};
     * auto result = parser.parseMultiple(values);
     *
     * double width = std::any_cast<double>(result["width"]);   // 3.14
     * double height = std::any_cast<double>(result["height"]); // 2.71
     * std::string name = std::any_cast<std::string>(result["name"]); // "rectangle"
     * ```
     */
    std::unordered_map<std::string, std::any> parseMultiple(const std::vector<std::string>& values) const override;

    /**
     * @brief Retrieves all registered output keys in registration order
     *
     * Returns a vector containing all keys that were registered via addValue() calls,
     * preserving the order in which they were added. This is useful for understanding
     * the expected structure of parseMultiple() results or for generating user
     * documentation about expected input format.
     *
     * @return Vector of strings containing all registered keys in the order they
     *         were added through addValue() calls. Empty vector if no keys registered.
     *
     * @note The returned vector reflects the current state of registered parsers.
     *       If called after partial configuration, it will only contain keys
     *       registered up to that point.
     *
     * @see addValue() for key registration
     * @see getExpectedValueCount() for the number of registered keys
     *
     * @example
     * ```cpp
     * CustomMultiValueParser parser;
     * parser.addValue("x", std::make_unique<DoubleParser>())
     *       .addValue("y", std::make_unique<DoubleParser>());
     *
     * auto keys = parser.getOutputKeys(); // Returns {"x", "y"}
     * ```
     */
    std::vector<std::string> getOutputKeys() const override;

    /**
     * @brief Gets the number of values expected by parseMultiple()
     *
     * Returns the count of registered key-parser pairs, which corresponds to the
     * exact number of string values that must be provided to parseMultiple().
     * This method is useful for validation before calling parseMultiple() or
     * for generating error messages about expected input format.
     *
     * @return Number of registered key-parser pairs (same as registered keys count)
     *
     * @note This count increases with each addValue() call and represents the
     *       current configuration state of the parser.
     *
     * @see addValue() for increasing the expected value count
     * @see parseMultiple() for the method that enforces this count
     *
     * @example
     * ```cpp
     * CustomMultiValueParser parser;
     * assert(parser.getExpectedValueCount() == 0); // No parsers registered
     *
     * parser.addValue("width", std::make_unique<DoubleParser>());
     * assert(parser.getExpectedValueCount() == 1); // One parser registered
     *
     * parser.addValue("height", std::make_unique<DoubleParser>());
     * assert(parser.getExpectedValueCount() == 2); // Two parsers registered
     * ```
     */
    size_t getExpectedValueCount() const override;
};