#pragma once

#include <any>
#include <string>
#include "IValueParser.h"

class StringParser : public IValueParser {
public:
    std::any parse(const std::string& value) const override {
        return value;
    }

    std::string getTypeName() const override { return "string"; }
};

