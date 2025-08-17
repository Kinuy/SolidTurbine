#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// Interface for resolving paths, allowing for flexible path handling
class IPathResolver {
public:
    virtual ~IPathResolver() = default;
    virtual fs::path resolvePath(const std::string& inputPath) const = 0;
};
