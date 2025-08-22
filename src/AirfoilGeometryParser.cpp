#include "AirfoilGeometryParser.h"

std::vector<std::string> AirfoilGeometryParser::tokenizeLine(const std::string& line, char delimiter = '\t') const {
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

bool AirfoilGeometryParser::isCoordinateLine(const std::vector<std::string>& tokens) const {
    return tokens.size() >= 3 && tokens[0] == "DEF";
}

bool AirfoilGeometryParser::isMarkerLine(const std::vector<std::string>& tokens) const {
    return tokens.size() >= 3 && tokens[0] == "MARKER";
}

bool AirfoilGeometryParser::isHeaderLine(const std::vector<std::string>& tokens) const {
    return tokens.size() >= 2 && (tokens[0] == "NAME" || tokens[0] == "RELDICKE");
}

std::unique_ptr<IStructuredData> AirfoilGeometryParser::parseFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open airfoil geometry file: " + filePath);
    }

    auto airfoilData = std::make_unique<AirfoilGeometryData>();
    std::string line;
    size_t lineNumber = 0;
	int idx = 0; // Index for coordinates, used to track order

    while (std::getline(file, line)) {
        ++lineNumber;

        // Skip empty lines
        if (line.empty()) continue;

        // Handle comment lines
        if (line[0] == '#') {
            airfoilData->addHeader(line);
            continue;
        }

        try {
            auto tokens = tokenizeLine(line);
            if (tokens.empty()) continue;

            // Parse different line types
            if (isHeaderLine(tokens)) {
                if (tokens[0] == "NAME") {
                    airfoilData->setName(tokens[1]);
                }
                else if (tokens[0] == "RELDICKE") {
                    airfoilData->setRelativeThickness(std::stod(tokens[1]));
                }
            }
            else if (isMarkerLine(tokens)) {
                std::string markerType = tokens[1];
                int markerIndex = std::stoi(tokens[2]);
                airfoilData->addMarker(markerType, markerIndex);
            }
            else if (isCoordinateLine(tokens)) {
				idx += 1; // Increment index for each coordinate
                double x = std::stod(tokens[1]);
                double y = std::stod(tokens[2]);
                airfoilData->addCoordinate(idx, x, y, 0, NULL, NULL);
            }

        }
        catch (const std::exception& e) {
            throw std::runtime_error("Error parsing airfoil geometry line " +
                std::to_string(lineNumber) + " in file " + filePath +
                ": " + e.what());
        }
    }

    if (airfoilData->getRowCount() == 0) {
        throw std::runtime_error("No valid airfoil coordinate data found in file: " + filePath);
    }

    return std::move(airfoilData);
}

std::vector<std::string> AirfoilGeometryParser::getSupportedExtensions() const {
    return { ".dat", ".txt", ".geo", ".coord" };
}

