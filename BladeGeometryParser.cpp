#include "BladeGeometryParser.h"


std::vector<std::string> BladeGeometryParser::tokenizeLine(const std::string& line, char delimiter = '\t') const {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    // If tab didn't work, try space separation
    if (tokens.size() <= 1 && delimiter == '\t') {
        return tokenizeLine(line, ' ');
    }

    return tokens;
}

std::unique_ptr<IStructuredData> BladeGeometryParser::parseFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open blade geometry file: " + filePath);
    }

    auto bladeData = std::make_unique<BladeGeometryData>();
    std::string line;
    size_t lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;

        // Skip empty lines
        if (line.empty()) continue;

        // Handle comment/header lines
        if (line[0] == '#') {
            bladeData->addHeader(line);
            continue;
        }

        try {
            auto tokens = tokenizeLine(line);
            if (tokens.empty()) continue;

            // Only process DEF lines (data rows)
            if (tokens[0] == "DEF") {
                BladeGeometryRow row(tokens);
                bladeData->addRow(row);
            }

        }
        catch (const std::exception& e) {
            throw std::runtime_error("Error parsing blade geometry line " +
                std::to_string(lineNumber) + ": " + e.what());
        }
    }

    if (bladeData->getRowCount() == 0) {
        throw std::runtime_error("No valid blade geometry data found in file: " + filePath);
    }

    return std::move(bladeData);
}

std::vector<std::string> BladeGeometryParser::getSupportedExtensions() const {
    return { ".dat", ".txt", ".csv" };
}