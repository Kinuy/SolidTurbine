#include "BladeGeometryParser.h"


// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

/**
 * @brief Scales relativeThickness from fractional [0–1] to percentage [1–100]
 *
 * Some BladeGeometry_E files store Dicke_rel. as a fraction (e.g. 0.21)
 * rather than as a percentage (e.g. 21.0).  Any value strictly less than 1
 * is assumed to be fractional and is multiplied by 100.
 *
 * @param value Raw parsed value from the Dicke_rel.[%] column
 * @return Value guaranteed to be in the percentage range [1, 100]
 */
static double normaliseRelThickness(double value) {
    return (value <= 1.0) ? value * 100.0 : value;
}

static double correctRelThicknessAtRootToCylinder(double value) {
    return (value < 1.0 && value > 0.99) ? 1.0 : value;
}


std::vector<std::string> BladeGeometryParser::tokenizeLine(const std::string& line, char delimiter) const {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        // Trim leading/trailing whitespace
        token.erase(0, token.find_first_not_of(" \t"));
        const auto last = token.find_last_not_of(" \t");
        if (last != std::string::npos)
            token.erase(last + 1);
        else
            token.clear();

        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    // If tab didn't split the line, fall back to space separation
    if (tokens.size() <= 1 && delimiter == '\t') {
        return tokenizeLine(line, ' ');
    }

    return tokens;
}

// ---------------------------------------------------------------------------

void BladeGeometryParser::parseStandardFormat(std::ifstream& stream,
                                              const std::string& /*filePath*/,
                                              BladeGeometryData& data,
                                              const std::string& firstLine) const
{
    auto processLine = [&](const std::string& line, size_t lineNumber) {
        if (line.empty()) return;

        if (line[0] == '#') {
            data.addHeader(line);
            return;
        }

        auto tokens = tokenizeLine(line);
        if (tokens.empty()) return;

        if (tokens[0] == "DEF") {
            try {
                BladeGeometrySection row(tokens);
                row.relativeThickness = normaliseRelThickness(row.relativeThickness);
                data.addRow(std::move(row));
            }
            catch (const std::exception& e) {
                throw std::runtime_error("Error parsing blade geometry line " +
                    std::to_string(lineNumber) + ": " + e.what());
            }
        }
    };

    size_t lineNumber = 1;
    processLine(firstLine, lineNumber);

    std::string line;
    while (std::getline(stream, line)) {
        ++lineNumber;
        processLine(line, lineNumber);
    }
}

// ---------------------------------------------------------------------------

void BladeGeometryParser::parseEFormat(std::ifstream& stream,
                                       const std::string& /*filePath*/,
                                       BladeGeometryData& data) const
{
    // Column layout (18 columns, tab-delimited):
    //  [0]  Section index   - discarded
    //  [1]  bladeRadius     [m]
    //  [2]  chord           [m]
    //  [3]  twist           [deg] -> converted to rad
    //  [4]  thicknessAbs_mm [mm]
    //  [5]  relativeThickness [%]
    //  [6]  xt4           [m]
    //  [7]  yt4           [m]
    //  [8]  trailingEdge_mm [mm]
    //  [9]  pcbaX      [m]
    // [10]  chordWithTES_m  [m]
    // [11]  gammaPrebend_deg [deg]
    // [12]  radiusPrebend_m  [m]
    // [13]  relThick001
    // [14]  relThick01
    // [15]  teAngle_deg     [deg]
    // [16]  addOns
    // [17]  airfoilName

    std::string line;
    size_t lineNumber = 1; // VERSIO line already consumed by caller
    bool headerSkipped = false; // the Sektion column-header line

    while (std::getline(stream, line)) {
        ++lineNumber;

        // Trim
        const auto first = line.find_first_not_of(" \t\r");
        if (first == std::string::npos) continue;
        const auto last = line.find_last_not_of(" \t\r");
        line = line.substr(first, last - first + 1);

        if (line.empty()) continue;

        auto tokens = tokenizeLine(line, '\t');
        if (tokens.empty()) continue;

        // FILE header line
        if (tokens[0] == "FILE") {
            if (tokens.size() > 1)
                data.setSourceFileName(tokens[1]);
            continue;
        }

        // Column-header line (first token non-numeric, e.g. "Sektion")
        if (!headerSkipped) {
            bool isNumeric = true;
            try { std::stod(tokens[0]); }
            catch (...) { isNumeric = false; }

            if (!isNumeric) {
                headerSkipped = true;
                continue;
            }
            headerSkipped = true;
        }

        // Data row - expect 18 columns
        if (tokens.size() < 18) {
            throw std::runtime_error("E-format blade geometry line " +
                std::to_string(lineNumber) + ": expected 18 columns, got " +
                std::to_string(tokens.size()));
        }

        try {
            BladeGeometrySection row;
            row.type              = "DEF";
            // col[0] = section index -- discard
            row.bladeRadius       = std::stod(tokens[1]);
            row.chord             = std::stod(tokens[2]);
            row.twist             = std::stod(tokens[3]) * std::numbers::pi / 180.0;
            row.thicknessAbs_mm   = std::stod(tokens[4]);
            row.relativeThickness = normaliseRelThickness(correctRelThicknessAtRootToCylinder(std::stod(tokens[5])));
            row.xt4             = std::stod(tokens[6]);
            row.yt4             = std::stod(tokens[7]);
            row.trailingEdge_mm   = std::stod(tokens[8]);
            row.prebend_mm        = std::stod(tokens[9]);
            row.chordWithTES_m    = std::stod(tokens[10]);
            row.gammaPrebend_deg  = std::stod(tokens[11]);
            row.radiusPrebend_m   = std::stod(tokens[12]);
            row.relThick001       = std::stod(tokens[13]);
            row.relThick01        = std::stod(tokens[14]);
            row.teAngle_deg       = std::stod(tokens[15]);
            row.addOns            = tokens[16];
            row.airfoilName       = tokens[17];
            // relativeTwistAxis, xt4, yt4 stay 0 (not present in E-format)
            row.pcbaX = row.prebend_mm / 1000.0;
            row.relativeTwistAxis = (1.0 + row.xt4 / row.chord) * 100.0;

            data.addRow(std::move(row));
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Error parsing E-format blade geometry line " +
                std::to_string(lineNumber) + ": " + e.what());
        }
    }
}


// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

std::unique_ptr<IStructuredData> BladeGeometryParser::parseFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open blade geometry file: " + filePath);
    }

    auto bladeData = std::make_unique<BladeGeometryData>();

    // Scan until the first non-blank, non-comment line to determine format
    std::string line;
    while (std::getline(file, line)) {
        // Trim
        const auto first = line.find_first_not_of(" \t\r");
        if (first == std::string::npos) continue;
        const auto last = line.find_last_not_of(" \t\r");
        line = line.substr(first, last - first + 1);

        if (line.empty()) continue;

        if (line[0] == '#') {
            bladeData->addHeader(line);
            continue;
        }

        // First meaningful line -- detect format by first token
        auto tokens = tokenizeLine(line, '\t');
            if (tokens.empty()) continue;

        if (tokens[0] == "VERSIO") {
            // E-format: store version string, delegate rest of file
            if (tokens.size() > 1)
                bladeData->setVersion(tokens[1]);
            parseEFormat(file, filePath, *bladeData);
        }
        else {
            // Standard format: re-process the already-read line
            parseStandardFormat(file, filePath, *bladeData, line);
        }
        break;
    }

    if (bladeData->getRowCount() == 0) {
        throw std::runtime_error("No valid blade geometry data found in file: " + filePath);
    }

    bladeData->normaliseRadii();

    return bladeData;
}

std::vector<std::string> BladeGeometryParser::getSupportedExtensions() const {
    return { ".dat", ".txt", ".csv" };
}
