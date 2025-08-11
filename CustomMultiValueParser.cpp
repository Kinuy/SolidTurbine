#include "CustomMultiValueParser.h"

CustomMultiValueParser& CustomMultiValueParser::addValue(const std::string& key, std::unique_ptr<IValueParser> parser) {
    keyParsers.emplace_back(key, std::move(parser));
    return *this;
}

std::unordered_map<std::string, std::any> CustomMultiValueParser::parseMultiple(const std::vector<std::string>& values) const {
    if (values.size() != keyParsers.size()) {
        throw std::invalid_argument("Expected " + std::to_string(keyParsers.size()) +
            " values, got " + std::to_string(values.size()));
    }

    std::unordered_map<std::string, std::any> result;
    for (size_t i = 0; i < values.size(); ++i) {
        const auto& [key, parser] = keyParsers[i];
        result[key] = parser->parse(values[i]);
    }

    return result;
}

std::vector<std::string> CustomMultiValueParser::getOutputKeys() const {
    std::vector<std::string> keys;
    for (const auto& [key, parser] : keyParsers) {
        keys.push_back(key);
    }
    return keys;
}

size_t CustomMultiValueParser::getExpectedValueCount() const { return keyParsers.size(); }
