#include "FilePathParser.h"


FilePathParser::FilePathParser() {
    // Register supported data file parsers
    dataFileParsers["blade_geometry"] = std::make_unique<BladeGeometryParser>();
    // Register supported file list parsers
    fileListParsers["airfoil_performance_files"] = std::make_unique<AirfoilPerformanceFileListParser>();
}

std::any FilePathParser::parse(const std::string& value) const {
    // Validate file exists
    if (!std::filesystem::exists(value)) {
        throw std::invalid_argument("File does not exist: " + value);
    }

    return value; // Return the validated file path
}

std::string FilePathParser::getTypeName() const { return "filepath"; }

// Parse the actual data file based on parameter name
std::unique_ptr<IStructuredData> FilePathParser::parseDataFile(const std::string& parameterName,
    const std::string& filePath) const {
    // Extract file type from parameter name (e.g., "blade_geometry_file" -> "blade_geometry")
    std::string fileType = extractFileType(parameterName);

    auto parserIt = dataFileParsers.find(fileType);
    if (parserIt == dataFileParsers.end()) {
        throw std::runtime_error("No parser available for file type: " + fileType);
    }

    return parserIt->second->parseFile(filePath);
}

std::string FilePathParser::extractFileType(const std::string& parameterName) const {
    // Remove "_file" suffix to get file type
    std::string type = parameterName;
    if (type.length() > 5 && type.substr(type.length() - 5) == "_file") {
        type = type.substr(0, type.length() - 5);
    }
    return type;
}

std::unique_ptr<IStructuredData> FilePathParser::parseFileListFile(const std::string& parameterName,
    const std::string& filePath) const {
    std::string fileType = extractFileListType(parameterName);

    auto parserIt = fileListParsers.find(fileType);
    if (parserIt == fileListParsers.end()) {
        throw std::runtime_error("No file list parser available for type: " + fileType);
    }

    return parserIt->second->parseFileList(filePath);
}

std::string FilePathParser::extractFileListType(const std::string& parameterName) const {
    // Extract type from patterns like "airfoil_performance_files_file"
    // Result should be "airfoil_performance_files"
    std::string type = parameterName;
    if (type.length() > 5 && type.substr(type.length() - 5) == "_file") {
        type = type.substr(0, type.length() - 5);
    }
    return type;
}

bool FilePathParser::isFileListParameter(const std::string& parameterName) const {
    return parameterName.find("_files_file") != std::string::npos;
}
