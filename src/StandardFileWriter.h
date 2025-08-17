#pragma once

#include <filesystem>
#include <string>
#include <iostream>
#include <stdexcept>
#include <fstream>

#include "IFileWriter.h"

namespace fs = std::filesystem;

// Responsible for standard file writing just taking file path and content as string
class StandardFileWriter : public IFileWriter {
public:
    bool writeFile(const fs::path& filePath, const std::string& content) const override;
};
