#pragma once

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "IDataFileParser.h"
#include "AirfoilGeometryData.h"
#include "AirfoilCoordinate.h"
#include "AirfoilMarker.h"
#include "AirfoilGeometryFileInfo.h"


/**
 * @brief File parser implementation for airfoil geometry coordinate data
 *
 * AirfoilGeometryParser implements IDataFileParser to read and parse airfoil
 * geometry files containing coordinate points, geometry markers, and metadata.
 * Handles flexible formatting with automatic delimiter detection and supports
 * various airfoil coordinate file formats.
 *
 * ## Supported File Format
 * - **Header Lines**: Metadata with keywords (NAME, RELDICKE)
 * - **Coordinate Lines**: "DEF" prefix followed by x,y coordinates
 * - **Marker Lines**: "MARKER" prefix with type and index information
 * - **Comments**: Lines starting with '#' (preserved as headers)
 * - **Delimiters**: Tab-separated preferred, space-separated fallback
 * - **Extensions**: .dat, .txt, .geo, .coord files
 *
 * ## Key Features
 * - Automatic delimiter detection and fallback
 * - Coordinate point extraction with validation
 * - Geometry marker parsing for special points
 * - Metadata extraction from header lines
 *
 * ## Typical File Structure
 * ```
 * # Airfoil geometry data
 * NAME NACA0012
 * RELDICKE 12.0
 * MARKER TEE 127
 * DEF 1.0000  0.0000
 * DEF 0.9500  0.0123
 * DEF 0.0000  0.0000
 * ```
 *
 * @see IDataFileParser for the base interface
 * @see AirfoilGeometryData for output data structure
 * @see AirfoilCoordinate for coordinate point structure
 * @see AirfoilMarker for geometry marker structure
 *
 * @example
 * ```cpp
 * AirfoilGeometryParser parser;
 * auto data = parser.parseFile("naca0012.dat");
 * auto airfoilData = static_cast<AirfoilGeometryData*>(data.get());
 * ```
 */
class AirfoilGeometryParser : public IDataFileParser {

private:

    /**
     * @brief Splits line into tokens using delimiter with automatic fallback
     * @param line Input line to tokenize
     * @param delimiter Primary delimiter character (default: tab)
     * @return Vector of trimmed, non-empty tokens
     * @note Falls back to space delimiter if tab produces ≤1 token
     */
    std::vector<std::string> tokenizeLine(const std::string& line, char delimiter) const;

    /**
     * @brief Checks if tokens represent an airfoil coordinate line
     * @param tokens Tokenized line to check
     * @return true if line starts with "DEF" and has ≥3 tokens
     */
    bool isCoordinateLine(const std::vector<std::string>& tokens) const;

    /**
     * @brief Checks if tokens represent a geometry marker line
     * @param tokens Tokenized line to check
     * @return true if line starts with "MARKER" and has ≥3 tokens
     */
    bool isMarkerLine(const std::vector<std::string>& tokens) const;

    /**
     * @brief Checks if tokens represent a header line with metadata
     * @param tokens Tokenized line to check
     * @return true if line starts with "NAME" or "RELDICKE" and has ≥2 tokens
     */
    bool isHeaderLine(const std::vector<std::string>& tokens) const;

public:

    /**
     * @brief Parses airfoil geometry file into structured data
     * @param filePath Path to airfoil geometry file
     * @return Unique pointer to AirfoilGeometryData with parsed coordinates and markers
     * @throws std::runtime_error if file cannot be opened, parsing fails, or no data found
     * @note Processes DEF lines as coordinates, MARKER lines as geometry markers
     */
    std::unique_ptr<IStructuredData> parseFile(const std::string& filePath) override;

    /**
     * @brief Gets list of supported file extensions
     * @return Vector of supported extensions: .dat, .txt, .geo, .coord
     */
    std::vector<std::string> getSupportedExtensions() const override;

};
