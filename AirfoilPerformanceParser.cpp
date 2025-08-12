#include "AirfoilPerformanceParser.h"


std::vector<std::string> AirfoilPerformanceParser::tokenizeLine(const std::string& line, char delimiter) const {
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

bool AirfoilPerformanceParser::isHeaderLine(const std::vector<std::string>& tokens) const {
    return tokens.size() >= 2 && (
        tokens[0] == "REFNUM" || tokens[0] == "XA" || tokens[0] == "THICK" ||
        tokens[0] == "REYN" || tokens[0] == "DEPANG" || tokens[0] == "NALPHA" ||
        tokens[0] == "NVALS"
        );
}

bool AirfoilPerformanceParser::isDataLine(const std::vector<std::string>& tokens) const {
    // Check if line has 4 numeric values (alpha, cl, cd, cm)
    if (tokens.size() < 4) return false;

    // Try to parse first token as number
    try {
        std::stod(tokens[0]);
        return true;
    }
    catch (...) {
        return false;
    }
}



std::unique_ptr<IStructuredData> AirfoilPerformanceParser::parseFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open airfoil performance file: " + filePath);
    }

    auto perfData = std::make_unique<AirfoilPerformanceData>();
    std::string line;
    size_t lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;

        // Skip empty lines
        if (line.empty()) continue;

        // Handle comment lines
        if (line[0] == '#') {
            perfData->addHeader(line);
            continue;
        }

        try {
            auto tokens = tokenizeLine(line);
            if (tokens.empty()) continue;

            // Parse header lines
            if (isHeaderLine(tokens)) {
                if (tokens[0] == "REFNUM") {
                    perfData->setName(tokens[1]);
                }
                else if (tokens[0] == "XA") {
                    perfData->setXa(std::stod(tokens[1]));
                }
                else if (tokens[0] == "THICK") {
                    perfData->setRelativeThickness(std::stod(tokens[1]));
                }
                else if (tokens[0] == "REYN") {
                    perfData->setReynoldsNumber(std::stod(tokens[1]));
                }
                else if (tokens[0] == "DEPANG") {
                    perfData->setDepang(std::stod(tokens[1]));
                }
                else if (tokens[0] == "NALPHA") {
                    perfData->setNAlpha(std::stoi(tokens[1]));
                }
                else if (tokens[0] == "NVALS") {
                    perfData->setNVals(std::stoi(tokens[1]));
                }
            }
            // Parse data lines
            else if (isDataLine(tokens)) {
                double alpha = std::stod(tokens[0]);
                double cl = std::stod(tokens[1]);
                double cd = std::stod(tokens[2]);
                double cm = (tokens.size() > 3) ? std::stod(tokens[3]) : 0.0;

                perfData->addPerformancePoint(alpha, cl, cd, cm);
            }

        }
        catch (const std::exception& e) {
            throw std::runtime_error("Error parsing airfoil performance line " +
                std::to_string(lineNumber) + " in file " + filePath +
                ": " + e.what());
        }
    }

    if (perfData->getRowCount() == 0) {
        throw std::runtime_error("No valid airfoil performance data found in file: " + filePath);
    }

    // Validate data consistency
    if (perfData->getNAlpha() > 0 &&
        static_cast<int>(perfData->getRowCount()) != perfData->getNAlpha()) {
        std::cout << "Warning: NALPHA (" << perfData->getNAlpha()
            << ") doesn't match actual data rows (" << perfData->getRowCount()
            << ") in file: " << filePath << std::endl;
    }

    return std::move(perfData);
}

std::vector<std::string> AirfoilPerformanceParser::getSupportedExtensions() const {
    return { ".dat", ".txt", ".perf", ".aero" };
}
