#include "ConfigurationParser.h"

ConfigurationParser::ConfigurationParser(
    ConfigurationSchema schema, 
    std::unique_ptr<IFileReader> reader)
    : 
    schema(std::move(schema)), 
    fileReader(std::move(reader)) 
{
}

ConfigurationParser::ParserMaps ConfigurationParser::createParserMaps() const {
    ParserMaps maps;

    for (const auto& param : schema.getParameters()) {
        if (param.isMultiValue) {
            maps.multiValueParsers[param.name] = param.multiParser.get();
            // Mark all output keys as tracked for validation
            for (const auto& outputKey : param.multiParser->getOutputKeys()) {
                maps.requiredKeys[outputKey] = param.required;
            }
        }
        else {
            maps.singleValueParsers[param.name] = param.parser.get();
            maps.requiredKeys[param.name] = param.required;
            // Check if this is a file-related parameter
            if (schema.isDataFileParameter(param.name)) {
                const FilePathParser* filePathParser =
                    dynamic_cast<const FilePathParser*>(param.parser.get());

                if (filePathParser && filePathParser->isFileListParameter(param.name)) {
                    maps.fileListParams.push_back(param.name);
                }
                else {
                    maps.dataFileParams.push_back(param.name);
                }
            }
        }
    }
    return maps;
}

void ConfigurationParser::validateRequiredParameters(const Configuration& config, const ParserMaps& parserMaps) const {
    for (const auto& [key, required] : parserMaps.requiredKeys) {
        if (required && !config.hasValue(key)) {
            throw std::runtime_error("Required parameter missing: " + key);
        }
    }
}

Configuration ConfigurationParser::parse() {
    Configuration config;
    auto parserMaps = createParserMaps();

    std::vector<std::string> lines = fileReader->readLines();

    // Parse each line
    for (size_t lineNum = 0; lineNum < lines.size(); ++lineNum) {
        try {
            auto parsed = LineParser::parseLine(lines[lineNum]);

            if (parsed.isEmpty || parsed.isComment) {
                continue;
            }

            // Check if this is a multi-value parameter
            auto multiParserIt = parserMaps.multiValueParsers.find(parsed.key);
            if (multiParserIt != parserMaps.multiValueParsers.end()) {
                // Parse multiple values
                auto results = multiParserIt->second->parseMultiple(parsed.values);
                for (const auto& [key, value] : results) {
                    config.setValue(key, value);
                }
                continue;
            }

            // Check if this is a single-value parameter
            auto singleParserIt = parserMaps.singleValueParsers.find(parsed.key);
            if (singleParserIt != parserMaps.singleValueParsers.end()) {
                if (parsed.hasMultipleValues) {
                    throw std::runtime_error("Parameter '" + parsed.key + "' expects single value, got multiple");
                }
                // Parse single value
                std::any value = singleParserIt->second->parse(parsed.value);
                config.setValue(parsed.key, std::move(value));
                continue;
            }

            // Unknown parameter
            throw std::runtime_error("Unknown configuration key: " + parsed.key);

        }
        catch (const std::exception& e) {
            throw std::runtime_error("Error on line " + std::to_string(lineNum + 1) + ": " + e.what());
        }
    }

    // Load external data files (both regular and file lists)
    loadDataFiles(config, parserMaps);

    // Validate required parameters
    validateRequiredParameters(config, parserMaps);

    return config;
}

void ConfigurationParser::loadDataFiles(Configuration& config, const ParserMaps& parserMaps) const {
    // Load regular data files
    for (const std::string& paramName : parserMaps.dataFileParams) {
        if (!config.hasValue(paramName)) {
            continue; // Skip if file path wasn't provided
        }

        try {
            std::string filePath = config.getFilePath(paramName);

            // Get the file path parser to load the data
            auto parserIt = parserMaps.singleValueParsers.find(paramName);
            if (parserIt != parserMaps.singleValueParsers.end()) {
                const FilePathParser* filePathParser =
                    dynamic_cast<const FilePathParser*>(parserIt->second);

                if (filePathParser) {
                    auto structuredData = filePathParser->parseDataFile(paramName, filePath);

                    // Store structured data with a key derived from parameter name
                    std::string dataKey = extractDataKey(paramName);
                    config.setStructuredData(dataKey, std::move(structuredData));
                }
            }

        }
        catch (const std::exception& e) {
            throw std::runtime_error("Error loading data file for parameter '" +
                paramName + "': " + e.what());
        }
    }
    // Load file list files
    for (const std::string& paramName : parserMaps.fileListParams) {
        if (!config.hasValue(paramName)) {
            continue; // Skip if file path wasn't provided
        }

        try {
            std::string filePath = config.getFilePath(paramName);

            // Get the file path parser to load the file list
            auto parserIt = parserMaps.singleValueParsers.find(paramName);
            if (parserIt != parserMaps.singleValueParsers.end()) {
                const FilePathParser* filePathParser =
                    dynamic_cast<const FilePathParser*>(parserIt->second);

                if (filePathParser) {
                    auto fileListData = filePathParser->parseFileListFile(paramName, filePath);

                    // Store file list data with a key derived from parameter name
                    std::string dataKey = extractFileListDataKey(paramName);
                    config.setStructuredData(dataKey, std::move(fileListData));
                }
            }

        }
        catch (const std::exception& e) {
            throw std::runtime_error("Error loading file list for parameter '" +
                paramName + "': " + e.what());
        }
    }
}

std::string ConfigurationParser::extractDataKey(const std::string& paramName) const {
    // Convert "blade_geometry_file" to "blade_geometry"
    std::string dataKey = paramName;
    if (dataKey.length() > 5 && dataKey.substr(dataKey.length() - 5) == "_file") {
        dataKey = dataKey.substr(0, dataKey.length() - 5);
    }
    return dataKey;
}

std::string ConfigurationParser::extractFileListDataKey(const std::string& paramName) const {
    // Convert "airfoil_performance_files_file" to "airfoil_performance_files"
    std::string dataKey = paramName;
    if (dataKey.length() > 5 && dataKey.substr(dataKey.length() - 5) == "_file") {
        dataKey = dataKey.substr(0, dataKey.length() - 5);
    }
    return dataKey;
}