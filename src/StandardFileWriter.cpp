#include "StandardFileWriter.h"

// Writes the content to a file at the specified path
bool StandardFileWriter::writeFile(const fs::path& filePath, const std::string& content) const {
    try {
        std::ofstream file(filePath);
        if (file.is_open()) {
            file << content;
            file.close();
            std::cout << "File written: " << filePath << std::endl;
            return true;
        }
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to write file: " << e.what() << std::endl;
        return false;
    }
}