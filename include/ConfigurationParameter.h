#pragma once
#include "IValueParser.h"
#include <memory>
#include <string>
#include "IMultiValueParser.h"

/**
 * @brief Configuration parameter definition with parsing rules and metadata
 *
 * ConfigurationParameter defines a single configuration parameter including
 * its name, parsing strategy, validation requirements, and documentation.
 * Supports both single-value and multi-value parameters through variant-like
 * design with appropriate parser types.
 *
 * ## Parameter Types
 * - **Single Value**: Uses IValueParser for simple type conversion
 * - **Multi Value**: Uses IMultiValueParser for complex parameter groups
 *
 * ## Key Features
 * - Flexible parsing through strategy pattern
 * - Required/optional parameter support
 * - Self-documenting with description field
 * - Type discrimination via isMultiValue flag
 *
 * @note Only one parser type is active based on isMultiValue flag
 * @note Constructor determines parameter type and sets appropriate parser
 *
 * @example
 * ```cpp
 * // Single value parameter
 * ConfigurationParameter speed("max_speed",
 *                             std::make_unique<DoubleParser>(),
 *                             true, "Maximum rotation speed");
 *
 * // Multi-value parameter
 * ConfigurationParameter range("angle_range",
 *                             std::make_unique<RangeParser>(...),
 *                             false, "Angle sweep range");
 * ```
 */
struct ConfigurationParameter {

    /**
     * @brief Parameter name/identifier
     */
    std::string name;

    /**
     * @brief Parser for single-value parameters (null if multi-value)
     */
    std::unique_ptr<IValueParser> parser;

    /**
     * @brief Parser for multi-value parameters (null if single-value)
     */
    std::unique_ptr<IMultiValueParser> multiParser;

    /**
     * @brief Whether parameter is mandatory
     */
    bool required;

    /**
     * @brief Human-readable parameter description
     */
    std::string description;

    /**
     * @brief Flag indicating parameter type (true = multi-value, false = single-value)
     */
    bool isMultiValue;

    /**
     * @brief Constructor for single-value parameters
     * @param n Parameter name
     * @param p Value parser (ownership transferred)
     * @param req Whether parameter is required (default: true)
     * @param desc Optional description
     */
    ConfigurationParameter(
        const std::string& n, 
        std::unique_ptr<IValueParser> p,
        bool req = true, 
        const std::string& desc = "")
        : 
        name(n), 
        parser(std::move(p)), 
        required(req), 
        description(desc), 
        isMultiValue(false) 
    {
    }

    /**
     * @brief Constructor for multi-value parameters
     * @param n Parameter name
     * @param mp Multi-value parser (ownership transferred)
     * @param req Whether parameter is required (default: true)
     * @param desc Optional description
     */
    ConfigurationParameter(
        const std::string& n, 
        std::unique_ptr<IMultiValueParser> mp,
        bool req = true, 
        const std::string& desc = "")
        : name(n), 
        multiParser(std::move(mp)), 
        required(req), 
        description(desc), 
        isMultiValue(true) 
    {
    }
};
