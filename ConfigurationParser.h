#pragma once
#include "ConfigurationSchema.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <vector>
#include <filesystem>
#include <iostream>

#include "IValueParser.h"
#include "IMultiValueParser.h"
#include "IFileReader.h"

#include "Configuration.h"
#include "LineParser.h"


// Main Parser that orchestrates the parsing process
class ConfigurationParser {
private:

    ConfigurationSchema schema;
    std::unique_ptr<IFileReader> fileReader;

    // Create lookup maps for both single and multi-value parsers
    struct ParserMaps {
        std::unordered_map<std::string, const IValueParser*> singleValueParsers;
        std::unordered_map<std::string, const IMultiValueParser*> multiValueParsers;
        std::unordered_map<std::string, bool> requiredKeys;
        std::vector<std::string> dataFileParams; // Track data file parameters
        std::vector<std::string> fileListParams; // Track file list parameters
    };

    ParserMaps createParserMaps() const;

    void validateRequiredParameters(const Configuration& config, const ParserMaps& parserMaps) const;

    void loadDataFiles(Configuration& config, const ParserMaps& parserMaps) const;

    std::string extractDataKey(const std::string& paramName) const;

    std::string extractFileListDataKey(const std::string& paramName) const;

    void loadIndividualAirfoilGeometries(Configuration& config, const FilePathParser* filePathParser,
        const std::string& fileListKey) const;

public:

    ConfigurationParser(ConfigurationSchema schema, std::unique_ptr<IFileReader> reader);

    Configuration parse();

};

