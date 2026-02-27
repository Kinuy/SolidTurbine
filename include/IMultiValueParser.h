#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <any>

/**
 * @brief Interface for parsing multiple string values into structured output
 *
 * IMultiValueParser defines the contract for parsing multiple related string
 * values into a structured key-value map. Enables complex parameter parsing
 * where multiple inputs map to named outputs through strategy pattern.
 *
 * ## Key Features
 * - **Multi-Value Input**: Processes vectors of related string values
 * - **Structured Output**: Maps inputs to named keys in result map
 * - **Validation Support**: Defines expected input count for validation
 * - **Flexible Mapping**: Different strategies for various parameter types
 *
 * ## Common Implementations
 * - **RangeParser**: Parses start/end/step values for ranges
 * - **CustomMultiValueParser**: Configurable multi-value parsing
 * - **CoordinateParser**: Parses x/y/z coordinate triplets
 *
 * @see RangeParser, CustomMultiValueParser for concrete implementations
 * @see ConfigurationParser for usage in configuration systems
 *
 * @example
 * ```cpp
 * std::unique_ptr<IMultiValueParser> parser =
 *     std::make_unique<RangeParser>("start", "end", "step", valueParser);
 *
 * std::vector<std::string> inputs = {"0.0", "10.0", "0.5"};
 * auto results = parser->parseMultiple(inputs);
 * ```
 */
class IMultiValueParser
{

public:
    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~IMultiValueParser() = default;

    /**
     * @brief Parses multiple string values into structured key-value map
     * @param values Vector of string values to parse
     * @return Map of output keys to parsed values (wrapped in std::any)
     * @throws std::invalid_argument if value count doesn't match expected count
     */
    virtual std::unordered_map<std::string, std::any> parseMultiple(const std::vector<std::string> &values) const = 0;

    /**
     * @brief Gets the output key names that will be generated
     * @return Vector of keys that appear in parseMultiple() result map
     */
    virtual std::vector<std::string> getOutputKeys() const = 0;

    /**
     * @brief Gets the expected number of input values
     * @return Number of string values expected by parseMultiple()
     */
    virtual size_t getExpectedValueCount() const = 0;
};
