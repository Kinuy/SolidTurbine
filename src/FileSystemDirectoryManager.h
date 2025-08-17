#pragma once

#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "IDirectoryManager.h"

namespace fs = std::filesystem;

// Responible for managing directories in the file system , do some checks etc..
class FileSystemDirectoryManager : public IDirectoryManager {
public:
    bool ensureDirectoryExists(const fs::path& path) const override;
};
