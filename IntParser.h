#pragma once

#include "IValueParser.h"
#include <any>
#include <string>
#include <stdexcept>

class IntParser : public IValueParser {
public:
    std::any parse(const std::string& value) const override;

    std::string getTypeName() const override;
};

