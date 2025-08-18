#pragma once

#include <string>
#include <memory>
#include <any>
#include <vector>
#include <unordered_map>
#include <stdexcept>

#include "IValueParser.h"
#include "IMultiValueParser.h"


/**
 * @brief Multi-value parser for range specifications (start, end, step)
 *
 * RangeParser implements IMultiValueParser to handle range definitions that
 * require exactly three values: start, end, and step. Uses configurable
 * output key names and dependency injection for flexible type handling.
 *
 * ## Key Features
 * - **Fixed Structure**: Always expects exactly 3 values in order
 * - **Configurable Keys**: Custom output key names for flexibility
 * - **Type Flexibility**: Uses injected parser for any numeric type
 * - **Range Semantics**: Designed for mathematical range specifications
 *
 * ## Common Use Cases
 * - Parameter sweeps and sensitivity analysis
 * - Loop generation (for loops with start/end/increment)
 * - Mathematical range definitions
 * - Configuration of iterative processes
 *
 * @see IMultiValueParser for the base interface
 * @see CustomMultiValueParser for variable-length parsing
 *
 * @note Output keys are customizable to match domain terminology
 *       (e.g., "min/max/delta" vs "start/end/step")
 *
 * @example
 * ```cpp
 * RangeParser parser("min", "max", "increment",
 *                   std::make_unique<DoubleParser>());
 *
 * std::vector<std::string> values = {"0.0", "10.0", "0.5"};
 * auto result = parser.parseMultiple(values);
 * double min = std::any_cast<double>(result["min"]);     // 0.0
 * double max = std::any_cast<double>(result["max"]);     // 10.0
 * double inc = std::any_cast<double>(result["increment"]); // 0.5
 * ```
 */
class RangeParser : public IMultiValueParser {

private:

    /**
     * @brief Output key name for the range start value
     */
    std::string startKey;

    /**
     * @brief Output key name for the range end value
     */
    std::string endKey;

    /**
     * @brief Output key name for the range step/increment value
     */
    std::string stepKey;

    /**
     * @brief Value parser for converting string inputs to target type
     *
     * Injected parser used to convert all three range values (start, end, step)
     * from strings to the desired numeric type. Ensures type consistency across
     * all range components.
     */
    std::unique_ptr<IValueParser> valueParser;

public:

    /**
     * @brief Constructor that configures range parsing with custom keys and value parser
     *
     * Creates a RangeParser that expects exactly 3 values (start, end, step) and
     * maps them to configurable output keys. Uses dependency injection for the
     * value parser to enable flexible type handling.
     *
     * @param startKey Output key name for the start value (e.g., "min", "from")
     * @param endKey Output key name for the end value (e.g., "max", "to")
     * @param stepKey Output key name for the step value (e.g., "increment", "delta")
     * @param parser Value parser for converting string inputs to target type
     *
     * @note Takes ownership of the parser via std::move()
     *
     * @example
     * ```cpp
     * RangeParser parser("start", "end", "step", std::make_unique<DoubleParser>());
     * ```
     */
    RangeParser(const std::string& startKey, const std::string& endKey,
        const std::string& stepKey, std::unique_ptr<IValueParser> parser);

    /**
     * @brief Parses three string values into a range specification map
     *
     * Takes exactly 3 string values representing start, end, and step values,
     * parses each using the configured value parser, and returns them mapped
     * to the configured output keys.
     *
     * @param values Vector containing exactly 3 strings: [start, end, step]
     * @return Map with parsed values keyed by configured key names
     * @throws std::invalid_argument if values.size() != 3
     * @throws Parser-specific exceptions if individual value parsing fails
     *
     * @example
     * ```cpp
     * std::vector<std::string> input = {"0.0", "10.0", "0.5"};
     * auto result = parser.parseMultiple(input);
     * double start = std::any_cast<double>(result["start"]);
     * ```
     */
    std::unordered_map<std::string, std::any> parseMultiple(const std::vector<std::string>& values) const override;

    /**
     * @brief Gets the configured output key names for range components
     *
     * Returns the three key names that will be used in the parseMultiple()
     * result map, in the order [startKey, endKey, stepKey].
     *
     * @return Vector containing the three configured key names
     */
    std::vector<std::string> getOutputKeys() const override;


    /**
     * @brief Gets the expected number of input values
     *
     * Range parsing always requires exactly 3 values (start, end, step).
     *
     * @return Always returns 3
     */
    size_t getExpectedValueCount() const override;

};