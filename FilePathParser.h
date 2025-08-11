#pragma once

#include<memory>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <stdexcept>
#include <any>

#include "IValueParser.h"
#include "BladeGeometryParser.h"
#include "IDataFileParser.h"


// Responsible for parsing file paths and delegating to specific data file parsers
class FilePathParser : public IValueParser {

private:

    std::unordered_map<std::string, std::unique_ptr<IDataFileParser>> dataFileParsers;

    std::string extractFileType(const std::string& parameterName) const;

public:

    FilePathParser();

    std::any parse(const std::string& value) const;

    std::string getTypeName() const;

    // Parse the actual data file based on parameter name
    std::unique_ptr<IStructuredData> parseDataFile(const std::string& parameterName,
        const std::string& filePath) const;

};

