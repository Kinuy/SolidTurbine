#pragma once

#include <string>
#include <any>

// Abstract base for value parsers
class IValueParser
{
public:
    virtual ~IValueParser() = default;
    virtual std::any parse(const std::string& value) const = 0;
    virtual std::string getTypeName() const = 0;
};

