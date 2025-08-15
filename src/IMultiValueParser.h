#pragma once
#include <unordered_map>
#include <string>
#include <any>

// Interface for multi-value parsing
class IMultiValueParser {
public:
    virtual ~IMultiValueParser() = default;
    virtual std::unordered_map<std::string, std::any> parseMultiple(const std::vector<std::string>& values) const = 0;
    virtual std::vector<std::string> getOutputKeys() const = 0;
    virtual size_t getExpectedValueCount() const = 0;
};
