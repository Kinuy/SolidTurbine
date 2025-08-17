#pragma once

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

// Interface for file writing operations
class IFileWriter {
public:
    virtual ~IFileWriter() = default;
    virtual bool writeFile(const fs::path& filePath, const std::string& content) const = 0;
};