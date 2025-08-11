#pragma once
#include "ConfigurationParameter.h"
#include "DoubleParser.h"
#include "IntParser.h"
#include "StringParser.h"
#include "BoolParser.h"
#include "RangeParser.h"

// Defines expected parameters
class ConfigurationSchema {
private:
    std::vector<ConfigurationParameter> parameters;

public:
    // Single value parameters
    ConfigurationSchema& add(
        const std::string& name, 
        std::unique_ptr<IValueParser> parser,
        bool required, 
        const std::string& description);

    // Multi-value parameters
    ConfigurationSchema& addMultiValue(
        const std::string& name, 
        std::unique_ptr<IMultiValueParser> parser,
        bool required, 
        const std::string& description);

    const std::vector<ConfigurationParameter>& getParameters() const;

    // Helper methods for common types
    ConfigurationSchema& addDouble(
        const std::string& name, 
        bool required, 
        const std::string& description);

    ConfigurationSchema& addInt(
        const std::string& name, 
        bool required, 
        const std::string& desc);

    ConfigurationSchema& addString(
        const std::string& name, 
        bool required, 
        const std::string& desc);

    ConfigurationSchema& addBool(
        const std::string& name, 
        bool required, 
        const std::string& desc);

    // Helper methods for multi-value patterns
    ConfigurationSchema& addRange(
        const std::string& triggerKey, 
        const std::string& startKey,
        const std::string& endKey, 
        const std::string& stepKey,
        bool required, 
        const std::string& desc);

    ConfigurationSchema& addIntRange(
        const std::string& triggerKey, 
        const std::string& startKey,
        const std::string& endKey, 
        const std::string& stepKey,
        bool required, 
        const std::string& desc);
};

