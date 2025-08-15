#include "DoubleParser.h"

std::any DoubleParser::parse(const std::string& value) const {
    try {
        return std::stod(value);
    }
    catch (const std::exception&) {
        throw std::invalid_argument("Cannot parse '" + value + "' as double");
    }
}

std::string DoubleParser::getTypeName() const { return "double"; }
