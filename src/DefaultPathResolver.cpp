#include "DefaultPathResolver.h"

DefaultPathResolver::DefaultPathResolver(const std::string& defaultDir)
    : defaultDirectory(defaultDir) {
}

fs::path DefaultPathResolver::resolvePath(const std::string& inputPath) const {
    if (inputPath.empty()) {
        return fs::current_path() / defaultDirectory;
    }

    fs::path path(inputPath);
    if (path.is_absolute()) {
        return path;
    }

    // For relative paths, still use default directory
    return fs::current_path() / defaultDirectory;
}
