#pragma once

#include <string>
#include <any>

/**
 * @brief Interface for parsing string values into typed data
 *
 * IValueParser defines the contract for converting string representations
 * to typed values through polymorphic implementations. Enables flexible
 * parsing strategies for different data types in configuration and data
 * processing systems.
 *
 * ## Key Features
 * - **Type Safety**: Each implementation handles specific data types
 * - **Strategy Pattern**: Pluggable parsing algorithms for different formats
 * - **Error Handling**: Implementations provide clear error reporting
 * - **Uniform Interface**: Common interface for all value types
 *
 * ## Common Implementations
 * - **DoubleParser**: Parses floating-point numbers
 * - **IntParser**: Parses integer values
 * - **StringParser**: Pass-through for string values
 * - **BoolParser**: Parses boolean representations
 * - **FilePathParser**: Validates and processes file paths
 *
 * @see ConfigurationParser for usage in configuration systems
 * @see DoubleParser, IntParser, StringParser for concrete implementations
 *
 * @example
 * ```cpp
 * std::unique_ptr<IValueParser> parser = std::make_unique<DoubleParser>();
 * std::any result = parser->parse("3.14159");
 * double value = std::any_cast<double>(result);
 * ```
 */
class IValueParser{

public:

    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~IValueParser() = default;

    /**
     * @brief Parses string value into typed data
     * @param value String representation to parse
     * @return std::any containing the parsed value of appropriate type
     * @throws std::invalid_argument if parsing fails or value format is invalid
     */
    virtual std::any parse(const std::string& value) const = 0;

    /**
     * @brief Gets the type name identifier for this parser
     * @return String identifying the data type handled by this parser
     */
    virtual std::string getTypeName() const = 0;

};

