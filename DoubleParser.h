#pragma once
#include <any>
#include <string>
#include <stdexcept>

#include "IValueParser.h"

// Concrete parsers for different types (SRP - Single Responsibility)
class DoubleParser : public IValueParser {
public:
    std::any parse(const std::string& value) const override;

    std::string getTypeName() const override;
};

