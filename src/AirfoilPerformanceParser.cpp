#include "AirfoilPerformanceParser.h"


// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

std::vector<std::string> AirfoilPerformanceParser::tokenizeLine(const std::string& line, char delimiter) const {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        token.erase(0, token.find_first_not_of(" \t"));
        const auto last = token.find_last_not_of(" \t");
        if (last != std::string::npos)
            token.erase(last + 1);
        else
            token.clear();
        if (!token.empty())
            tokens.push_back(token);
        }

    // Tab delimiter produced no split — fall back to space
    if (tokens.size() <= 1 && delimiter == '\t')
        return tokenizeLine(line, ' ');

    return tokens;
}

bool AirfoilPerformanceParser::isHeaderLine(const std::vector<std::string>& tokens) const {
    return tokens.size() >= 2 && (
        tokens[0] == "REFNUM" || tokens[0] == "XA"     || tokens[0] == "THICK" ||
        tokens[0] == "REYN"   || tokens[0] == "DEPANG" || tokens[0] == "NALPHA" ||
        tokens[0] == "NVALS");
}

bool AirfoilPerformanceParser::isDataLine(const std::vector<std::string>& tokens) const {
    if (tokens.size() < 4) return false;
    try { std::stod(tokens[0]); return true; }
    catch (...) { return false; }
}

AirfoilPerformanceParser::PolarFormat
AirfoilPerformanceParser::detectFormat(const std::string& firstContentLine) const {
    // Tecplot files start with   TITLE=   (case-insensitive check on first 6 chars)
    if (firstContentLine.size() >= 6) {
        std::string prefix = firstContentLine.substr(0, 6);
        std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::toupper);
        if (prefix == "TITLE=")
            return PolarFormat::Tecplot;
    }
    return PolarFormat::Native;
}

bool AirfoilPerformanceParser::extractAuxDataValue(const std::string& line,
                                                    const std::string& key,
                                                    std::string& value) const {
    // Line format:  AUXDATA key="value"
    // Find the key (case-insensitive)
    std::string lineLower = line;
    std::string keyLower  = key;
    std::transform(lineLower.begin(), lineLower.end(), lineLower.begin(), ::tolower);
    std::transform(keyLower.begin(),  keyLower.end(),  keyLower.begin(),  ::tolower);

    auto keyPos = lineLower.find(keyLower);
    if (keyPos == std::string::npos) return false;

    // After the key there should be  ="value"
    auto eqPos = line.find('=', keyPos);
    if (eqPos == std::string::npos) return false;

    auto q1 = line.find('"', eqPos);
    auto q2 = line.find('"', q1 + 1);
    if (q1 == std::string::npos || q2 == std::string::npos) return false;

    value = line.substr(q1 + 1, q2 - q1 - 1);
        return true;
    }


// ---------------------------------------------------------------------------
// Native keyword format
// ---------------------------------------------------------------------------

void AirfoilPerformanceParser::parseNativeFormat(std::ifstream& file,
                                                  const std::string& filePath,
                                                  const std::string& firstLine,
                                                  AirfoilPolarData& perfData) const
{
    double mach    = 0.0;
    double reynolds = 0.0;
    size_t lineNumber = 0;

    auto processLine = [&](const std::string& line) {
        ++lineNumber;
        if (line.empty()) return;

        if (line[0] == '#') {
            perfData.addHeader(line);
            return;
        }

        auto tokens = tokenizeLine(line);
        if (tokens.empty()) return;

        if (isHeaderLine(tokens)) {
            if      (tokens[0] == "REFNUM") { perfData.setName(tokens[1]); }
            else if (tokens[0] == "XA")     { perfData.setXa(std::stod(tokens[1])); }
            else if (tokens[0] == "THICK")  { perfData.setRelativeThickness(std::stod(tokens[1])); }
            else if (tokens[0] == "REYN")   { reynolds = std::stod(tokens[1]); }
            else if (tokens[0] == "DEPANG") { perfData.setDepang(std::stod(tokens[1])); }
            else if (tokens[0] == "NALPHA") { perfData.setNAlpha(std::stoi(tokens[1])); }
            else if (tokens[0] == "NVALS")  { perfData.setNVals(std::stoi(tokens[1])); }
        }
        else if (isDataLine(tokens)) {
            double alpha = std::stod(tokens[0]) * std::numbers::pi / 180.0;
            double cl    = std::stod(tokens[1]);
            double cd    = std::stod(tokens[2]);
            double cm    = (tokens.size() > 3) ? std::stod(tokens[3]) : 0.0;
            perfData.addPolarPoint(AirfoilOperationCondition(reynolds, mach, alpha),
                                   AirfoilAeroCoefficients(cl, cd, cm));
        }
    };

    // Re-process the first line that was already read during format detection
    try { processLine(firstLine); }
    catch (const std::exception& e) {
        throw std::runtime_error("Error parsing airfoil performance line 1 in file "
                                 + filePath + ": " + e.what());
    }

    std::string line;
    while (std::getline(file, line)) {
        try { processLine(line); }
        catch (const std::exception& e) {
            throw std::runtime_error("Error parsing airfoil performance line "
                                     + std::to_string(lineNumber) + " in file "
                                     + filePath + ": " + e.what());
    }
    }

    // Validate NALPHA consistency
    if (perfData.getNAlpha() > 0 &&
        static_cast<int>(perfData.getRowCount()) != perfData.getNAlpha()) {
        std::cout << "Warning: NALPHA (" << perfData.getNAlpha()
                  << ") doesn't match actual data rows (" << perfData.getRowCount()
                  << ") in file: " << filePath << std::endl;
    }
}


// ---------------------------------------------------------------------------
// Tecplot ASCII format
// ---------------------------------------------------------------------------

void AirfoilPerformanceParser::parseTecplotFormat(std::ifstream& file,
                                                   const std::string& filePath,
                                                   const std::string& firstLine,
                                                   AirfoilPolarData& perfData) const
{
    double reynolds  = 0.0;
    double mach      = 0.0;
    bool   dataPhase = false;   // true once we have passed all header/AUXDATA lines
    size_t lineNumber = 1;      // firstLine was line 1

    // ---- helper: strip outer quotes from a Tecplot string value ----------
    auto stripQuotes = [](const std::string& s) -> std::string {
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
            return s.substr(1, s.size() - 2);
        return s;
    };

    // ---- process TITLE= line (already read as firstLine) -----------------
    // Extract polar name from:  TITLE="Polarname.pol"
    {
        auto eq = firstLine.find('=');
        if (eq != std::string::npos) {
            std::string raw = firstLine.substr(eq + 1);
            // trim whitespace
            raw.erase(0, raw.find_first_not_of(" \t"));
            perfData.setName(stripQuotes(raw));
        }
    }

    // ---- read remaining lines --------------------------------------------
    std::string line;
    while (std::getline(file, line)) {
        ++lineNumber;

        // Trim leading/trailing whitespace (handles \r\n)
        const auto first = line.find_first_not_of(" \t\r");
        if (first == std::string::npos) continue;
        const auto last  = line.find_last_not_of(" \t\r");
        line = line.substr(first, last - first + 1);
        if (line.empty()) continue;

        // ---- comment lines -----------------------------------------------
        if (line[0] == '#') {
            perfData.addHeader(line);
            continue;
        }

        // ---- Once in data phase, everything is a data row ----------------
        if (dataPhase) {
            try {
                auto tokens = tokenizeLine(line, ' ');
                if (tokens.size() < 4) continue;

                double alpha = std::stod(tokens[0]) * std::numbers::pi / 180.0;
                double cl    = std::stod(tokens[1]);
                double cd    = std::stod(tokens[2]);
                double cm    = std::stod(tokens[3]);

                perfData.addPolarPoint(
                    AirfoilOperationCondition(reynolds, mach, alpha),
                    AirfoilAeroCoefficients(cl, cd, cm));
                }
            catch (const std::exception& e) {
                throw std::runtime_error("Error parsing Tecplot data line "
                    + std::to_string(lineNumber) + " in file "
                    + filePath + ": " + e.what());
            }
            continue;
        }

        // ---- uppercase prefix for keyword matching -----------------------
        std::string lineUpper = line;
        std::transform(lineUpper.begin(), lineUpper.end(), lineUpper.begin(), ::toupper);

        // VARIABLES line — validate column order, otherwise ignore
        if (lineUpper.rfind("VARIABLES", 0) == 0) {
            perfData.addHeader("# VARIABLES: " + line);
            continue;
        }

        // ZONE line — extract point count (I=) and zone title (T=)
        if (lineUpper.rfind("ZONE", 0) == 0) {
            // Extract I=<n>
            auto iPos = lineUpper.find("I=");
            if (iPos != std::string::npos) {
                try {
                    size_t after = iPos + 2;
                    // skip spaces
                    while (after < line.size() && line[after] == ' ') ++after;
                    size_t end = after;
                    while (end < line.size() && (std::isdigit(line[end]) || line[end] == '-')) ++end;
                    int pointCount = std::stoi(line.substr(after, end - after));
                    perfData.setNAlpha(pointCount);
                }
                catch (...) { /* non-fatal — NALPHA check is just a warning */ }
            }

            // Extract T="zoneName" — use as name only if TITLE was empty
            {
                auto tPos = lineUpper.find(" T=");
                if (tPos == std::string::npos) tPos = lineUpper.find("\tT=");
                if (tPos != std::string::npos) {
                    auto q1 = line.find('"', tPos);
                    auto q2 = (q1 != std::string::npos) ? line.find('"', q1 + 1) : std::string::npos;
                    if (q1 != std::string::npos && q2 != std::string::npos) {
                        std::string zoneName = line.substr(q1 + 1, q2 - q1 - 1);
                        if (perfData.getName().empty())
                            perfData.setName(zoneName);
                    }
                }
            }
            continue;
        }

        // AUXDATA lines
        if (lineUpper.rfind("AUXDATA", 0) == 0) {
            std::string val;

            // Reynolds number
            if (extractAuxDataValue(line, "renum", val)) {
                try { reynolds = std::stod(val); }
                catch (...) {}
            }

            // Mach number  ("None" → leave at 0.0)
            if (extractAuxDataValue(line, "ma", val) && val != "None" && val != "none") {
                try { mach = std::stod(val); }
                catch (...) {}
            }

            // Relative thickness in %
            if (extractAuxDataValue(line, "relThick", val)) {
                try { perfData.setRelativeThickness(std::stod(val) * 100.0); }
                catch (...) {}
            }

            // Airfoil name — use as fallback if polar name not yet set
            if (extractAuxDataValue(line, "airfoil", val)) {
                if (perfData.getName().empty() || perfData.getName() == "unnamed")
                    perfData.setName(val);
                }

            // Preserve remaining AUXDATA as headers for traceability
            // (stallAngle, negStallAngle, designAngle, opt, tool, addons, runnum, ar, path)
            perfData.addHeader("# " + line);
            continue;
        }

        // Any other non-numeric line in the header block — store as header
        bool isNumericStart = false;
        if (!line.empty()) {
            try { std::stod(tokenizeLine(line, ' ')[0]); isNumericStart = true; }
            catch (...) {}
            }

        if (!isNumericStart) {
            perfData.addHeader("# " + line);
            continue;
        }

        // First numeric line — enter data phase and process immediately
        dataPhase = true;
        try {
            auto tokens = tokenizeLine(line, ' ');
            if (tokens.size() >= 4) {
                double alpha = std::stod(tokens[0]) * std::numbers::pi / 180.0;
                double cl    = std::stod(tokens[1]);
                double cd    = std::stod(tokens[2]);
                double cm    = std::stod(tokens[3]);
                perfData.addPolarPoint(
                    AirfoilOperationCondition(reynolds, mach, alpha),
                    AirfoilAeroCoefficients(cl, cd, cm));
            }
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Error parsing Tecplot data line "
                + std::to_string(lineNumber) + " in file "
                + filePath + ": " + e.what());
        }
    }

    // Validate point count consistency
    if (perfData.getNAlpha() > 0 &&
        static_cast<int>(perfData.getRowCount()) != perfData.getNAlpha()) {
        std::cout << "Warning: ZONE I=" << perfData.getNAlpha()
                  << " doesn't match actual data rows (" << perfData.getRowCount()
                  << ") in file: " << filePath << std::endl;
    }
}


// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

std::unique_ptr<IStructuredData> AirfoilPerformanceParser::parseFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open())
        throw std::runtime_error("Cannot open airfoil performance file: " + filePath);

    auto perfData = std::make_unique<AirfoilPolarData>();

    // Find first non-empty, non-comment line for format detection
    std::string firstLine;
    while (std::getline(file, firstLine)) {
        // Trim
        const auto f = firstLine.find_first_not_of(" \t\r");
        if (f == std::string::npos) continue;
        const auto l = firstLine.find_last_not_of(" \t\r");
        firstLine = firstLine.substr(f, l - f + 1);
        if (firstLine.empty()) continue;
        if (firstLine[0] == '#') { perfData->addHeader(firstLine); continue; }
        break;
    }

    if (firstLine.empty())
        throw std::runtime_error("No content found in file: " + filePath);

    PolarFormat fmt = detectFormat(firstLine);

    if (fmt == PolarFormat::Tecplot)
        parseTecplotFormat(file, filePath, firstLine, *perfData);
    else
        parseNativeFormat(file, filePath, firstLine, *perfData);

    if (perfData->getRowCount() == 0)
        throw std::runtime_error("No valid airfoil performance data found in file: " + filePath);

    return perfData;
}

std::vector<std::string> AirfoilPerformanceParser::getSupportedExtensions() const {
    return { ".dat", ".txt", ".perf", ".aero", ".pol" };
}
