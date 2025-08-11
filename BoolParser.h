#pragma once

#include <algorithm>

#include "IValueParser.h"
#include <stdexcept>



class BoolParser : public IValueParser {
public:
    std::any parse(const std::string& value) const;

    std::string getTypeName() const;
};

