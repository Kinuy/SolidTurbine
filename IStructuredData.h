#pragma once

#include <string>

// Base class for structured data (e.g., tables, arrays, etc.)
class IStructuredData {
public:
    virtual ~IStructuredData() = default;
    virtual std::string getTypeName() const = 0;
    virtual size_t getRowCount() const = 0;
};