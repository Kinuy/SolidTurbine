#pragma once

#include <memory>
#include <vector>
#include <string>

#include "IStructuredData.h"

// Interface for parsing external data files
class IDataFileParser {
public:
    virtual ~IDataFileParser() = default;
    virtual std::unique_ptr<IStructuredData> parseFile(const std::string& filePath) = 0;
    virtual std::vector<std::string> getSupportedExtensions() const = 0;
};