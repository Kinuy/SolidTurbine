#include "FlexiblePathResolver.h"


FlexiblePathResolver::FlexiblePathResolver(const std::string& defaultDir)
    : defaultDirectory(defaultDir) {
}

// Resolves the path based on the input, using the default directory if necessary
fs::path FlexiblePathResolver::resolvePath(const std::string& inputPath) const {
    if (inputPath.empty()) {
        return fs::current_path() / defaultDirectory;
    }

    fs::path path(inputPath);
    if (path.is_absolute()) {
        return path;
    }

    // For relative paths, use them under default directory
    return fs::current_path() / defaultDirectory / path;
}