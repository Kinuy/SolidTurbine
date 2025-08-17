#pragma once

#include <filesystem>
#include <string>

#include "IPathResolver.h"

namespace fs = std::filesystem;

// Responsible for flexible path resolving, allowing for custom default directories
class FlexiblePathResolver : public IPathResolver {
private:
    std::string defaultDirectory;

public:
    explicit FlexiblePathResolver(const std::string& defaultDir = "export");
    fs::path resolvePath(const std::string& inputPath) const override;
};
