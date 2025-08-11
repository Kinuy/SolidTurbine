#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

#include "IValueParser.h"
#include "IMultiValueParser.h"



// Generic multi-value parser with custom key mapping
class CustomMultiValueParser : public IMultiValueParser {

private:

    std::vector<std::pair<std::string, std::unique_ptr<IValueParser>>> keyParsers;

public:

    CustomMultiValueParser& addValue(const std::string& key, std::unique_ptr<IValueParser> parser);

    std::unordered_map<std::string, std::any> parseMultiple(const std::vector<std::string>& values) const override;

    std::vector<std::string> getOutputKeys() const override;

    size_t getExpectedValueCount() const override;
};