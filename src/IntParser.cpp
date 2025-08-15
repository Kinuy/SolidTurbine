#include "IntParser.h"

std::any IntParser::parse(const std::string& value) const {
    try {
        return std::stoi(value);
    }
    catch (const std::exception&) {
        throw std::invalid_argument("Cannot parse '" + value + "' as int");
    }
}

std::string IntParser::getTypeName() const { return "int"; }
