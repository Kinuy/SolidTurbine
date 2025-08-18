#include "StringParser.h"

std::any StringParser::parse(const std::string& value) const {
    return value;
}

std::string StringParser::getTypeName() const { 
    return "string"; 
}

