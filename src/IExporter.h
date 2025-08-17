#pragma once

#include <string>

// Interface for exporting data to files
class IExporter {
public:
    virtual ~IExporter() = default;
    virtual bool exportData(const std::string& path, const std::string& filename, const std::string& data) = 0;
};