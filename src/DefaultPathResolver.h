#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

#include "IPathResolver.h"


// Responsible for flexible path resolving, allowing for custom default directories
class DefaultPathResolver : public IPathResolver {
private:
    std::string defaultDirectory;

public:
    explicit DefaultPathResolver(const std::string& defaultDir = "export");
    fs::path resolvePath(const std::string& inputPath) const override;
};

