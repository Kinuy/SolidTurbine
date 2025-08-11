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

    // Validate required parameters
    validateRequiredParameters(config, parserMaps);

    return config;
}