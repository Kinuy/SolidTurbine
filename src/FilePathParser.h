#pragma once

#include<memory>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <stdexcept>
#include <any>

#include "IValueParser.h"
#include "BladeGeometryParser.h"
#include "AirfoilPerformanceFileListParser.h"
#include "IDataFileParser.h"
#include "IFileListParser.h"
#include "IStructuredData.h"
#include "AirfoilGeometryFileListParser.h"
#include "AirfoilGeometryParser.h"
#include "AirfoilPerformanceParser.h"


// Responsible for parsing file paths and delegating to specific data file parsers
class FilePathParser : public IValueParser {

private:

    std::unordered_map<std::string, std::unique_ptr<IDataFileParser>> dataFileParsers;

    std::unordered_map<std::string, std::unique_ptr<IFileListParser>> fileListParsers;

    std::string extractFileType(const std::string& parameterName) const;

    std::string extractFileListType(const std::string& parameterName) const;

public:

    FilePathParser();

    std::any parse(const std::string& value) const;

    std::string getTypeName() const;

    // Check if parameter is a file list type
    bool isFileListParameter(const std::string& parameterName) const;

    // Parse the actual data file based on parameter name
    std::unique_ptr<IStructuredData> parseDataFile(const std::string& parameterName,
        const std::string& filePath) const;

    std::unique_ptr<IStructuredData> parseFileListFile(const std::string& parameterName,
        const std::string& filePath) const;

    std::unique_ptr<IStructuredData> parseIndividualFile(const std::string& filePath,
        const std::string& fileType) const;

};

