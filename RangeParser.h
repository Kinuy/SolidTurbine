#pragma once

#include <string>
#include <memory>
#include <any>
#include <vector>
#include <unordered_map>
#include <stdexcept>

#include "IValueParser.h"
#include "IMultiValueParser.h"


// Range parser for start / end / step patterns
class RangeParser : public IMultiValueParser {
private:
    std::string startKey;
    std::string endKey;
    std::string stepKey;
    std::unique_ptr<IValueParser> valueParser;

public:
    RangeParser(const std::string& startKey, const std::string& endKey,
        const std::string& stepKey, std::unique_ptr<IValueParser> parser);

    std::unordered_map<std::string, std::any> parseMultiple(const std::vector<std::string>& values) const override;

    std::vector<std::string> getOutputKeys() const override;

    size_t getExpectedValueCount() const override;
};