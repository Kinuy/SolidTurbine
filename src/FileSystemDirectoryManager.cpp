#include "FileSystemDirectoryManager.h"

// Ensures that the specified directory exists, creating it if necessary
bool FileSystemDirectoryManager::ensureDirectoryExists(const fs::path& path) const {
    try {
        if (!fs::exists(path)) {
            fs::create_directories(path);
            std::cout << "Created directory: " << path << std::endl;
            return true;
        }
        return true;
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to create directory: " << e.what() << std::endl;
        return false;
    }
}
