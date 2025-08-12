#pragma once

#include <memory>
#include <string>
#include <vector>

#include "IStructuredData.h"


// Interface for parsing files that contain lists of other files
class IFileListParser {
public:
    virtual ~IFileListParser() = default;
    virtual std::unique_ptr<IStructuredData> parseFileList(const std::string& indexFilePath) = 0;
    virtual std::vector<std::string> getSupportedExtensions() const = 0;
};