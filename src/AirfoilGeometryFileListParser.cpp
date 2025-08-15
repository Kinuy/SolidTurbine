#include "AirfoilGeometryFileListParser.h"

std::string AirfoilGeometryFileListParser::trimLine(const std::string& line) const {
    size_t start = line.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";

    size_t end = line.find_last_not_of(" \t\r\n");
    return line.substr(start, end - start + 1);
}

bool AirfoilGeometryFileListParser::isAbsolutePath(const std::string& path) const {
    return std::filesystem::path(path).is_absolute();
}

std::string AirfoilGeometryFileListParser::resolveRelativePath(const std::string& relativePath, const std::string& indexFilePath) const {
    if (isAbsolutePath(relativePath)) {
        return relativePath;
    }

    // Resolve relative to the index file's directory
    std::filesystem::path indexDir = std::filesystem::path(indexFilePath).parent_path();
    std::filesystem::path resolvedPath = indexDir / relativePath;
    return resolvedPath.string();
}

std::unique_ptr<IStructuredData> AirfoilGeometryFileListParser::parseFileList(const std::string& indexFilePath) {
    std::ifstream file(indexFilePath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open airfoil geometry file list: " + indexFilePath);
    }

    auto fileListData = std::make_unique<AirfoilGeometryFileListData>();
    std::string line;
    size_t lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;

        std::string trimmed = trimLine(line);
        if (trimmed.empty()) continue;

        // Handle comment/header lines
        if (trimmed[0] == '#') {
            fileListData->addHeader(trimmed);
            continue;
        }

        try {
            // This should be a file path
            std::string resolvedPath = resolveRelativePath(trimmed, indexFilePath);
            fileListData->addFilePath(resolvedPath);

        }
        catch (const std::exception& e) {
            throw std::runtime_error("Error processing geometry file path on line " +
                std::to_string(lineNumber) + ": " + e.what());
        }
    }

    if (fileListData->getRowCount() == 0) {
        throw std::runtime_error("No geometry file paths found in: " + indexFilePath);
    }

    // Validate that at least some files exist
    if (fileListData->getValidFileCount() == 0) {
        auto missingFiles = fileListData->getMissingFiles();
        std::string errorMsg = "No valid airfoil geometry files found. Missing files:\n";
        for (const auto& missing : missingFiles) {
            errorMsg += "  - " + missing + "\n";
        }
        throw std::runtime_error(errorMsg);
    }

    return std::move(fileListData);
}

std::vector<std::string> AirfoilGeometryFileListParser::getSupportedExtensions() const {
    return { ".txt", ".dat", ".list", ".files", ".geo" };
}
