#include "RangeParser.h"

RangeParser::RangeParser(
    const std::string& startKey, 
    const std::string& endKey,
    const std::string& stepKey, 
    std::unique_ptr<IValueParser> parser)
    : 
    startKey(startKey), 
    endKey(endKey), 
    stepKey(stepKey), 
    valueParser(std::move(parser)) 
{
}

std::unordered_map<std::string, std::any> RangeParser::parseMultiple(const std::vector<std::string>& values) const {
    if (values.size() != 3) {
        throw std::invalid_argument("Range parser expects exactly 3 values (start, end, step)");
    }

    std::unordered_map<std::string, std::any> result;
    result[startKey] = valueParser->parse(values[0]);
    result[endKey] = valueParser->parse(values[1]);
    result[stepKey] = valueParser->parse(values[2]);

    return result;
}

std::vector<std::string> RangeParser::getOutputKeys() const {
    return { startKey, endKey, stepKey };
}

size_t RangeParser::getExpectedValueCount() const {
    return 3; 
}
