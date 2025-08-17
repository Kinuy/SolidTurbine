#pragma once

#include <filesystem>

namespace fs = std::filesystem;

// Interface for managing directories in the file system
class IDirectoryManager {
public:
    virtual ~IDirectoryManager() = default;
    virtual bool ensureDirectoryExists(const fs::path& path) const = 0;
};