#include "BoolParser.h"

std::any BoolParser::parse(const std::string& value) const {
    std::string lower_value = value;
    std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);

    if (lower_value == "true" || lower_value == "1") return true;
    if (lower_value == "false" || lower_value == "0") return false;

    throw std::invalid_argument("Cannot parse '" + value + "' as bool");
}

std::string BoolParser::getTypeName() const { return "bool"; }