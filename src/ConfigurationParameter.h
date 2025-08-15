#pragma once
#include "IValueParser.h"
#include <memory>
#include <string>
#include "IMultiValueParser.h"

// Defines expected parameters
struct ConfigurationParameter {
    std::string name;
    std::unique_ptr<IValueParser> parser;
    std::unique_ptr<IMultiValueParser> multiParser;
    bool required;
    std::string description;
    bool isMultiValue;

    // Single value constructor
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

    // Multi-value constructor
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
