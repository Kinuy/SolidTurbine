#pragma once

#include "IDataFileParser.h"
#include "IStructuredData.h"
#include "BladeGeometryData.h"

#include <sstream>
#include <fstream>

/**
 * @brief File parser implementation for blade geometry data files
 *
 * BladeGeometryParser implements IDataFileParser to read and parse blade
 * geometry files containing radial section data. Handles flexible formatting
 * with automatic delimiter detection and robust error handling.
 *
 * ## Supported Format
 * - **Header Lines**: Comments starting with '#' (preserved as headers)
 * - **Data Lines**: "DEF" prefix followed by section parameters
 * - **Delimiters**: Tab-separated preferred, space-separated fallback
 * - **Extensions**: .dat, .txt, .csv files
 *
 * ## Key Features
 * - Automatic delimiter detection (tab → space fallback)
 * - Whitespace trimming and empty line skipping
 * - Line-by-line error reporting with line numbers
 * - Header preservation for metadata
 *
 * ## Typical File Format
 * ```
 * # Blade geometry data
 * # Radius  Chord  Twist  Thickness
 * DEF 0.2   0.15   45.0   0.12
 * DEF 0.5   0.12   25.0   0.10
 * ```
 *
 * @see IDataFileParser for the base interface
 * @see BladeGeometryData for output data structure
 * @see BladeGeometrySection for individual section data
 *
 * @example
 * ```cpp
 * BladeGeometryParser parser;
 * auto data = parser.parseFile("blade.dat");
 * auto bladeData = static_cast<BladeGeometryData*>(data.get());
 * ```
 */
class BladeGeometryParser : public IDataFileParser {

private:

    /**
     * @brief Splits a line into tokens using delimiter with automatic fallback
     * @param line Input line to tokenize
     * @param delimiter Primary delimiter character (default: tab)
     * @return Vector of trimmed, non-empty tokens
     * @note Automatically falls back to space delimiter if tab produces ≤1 token
     */
    std::vector<std::string> tokenizeLine(const std::string& line, char delimiter) const;

public:

    /**
     * @brief Parses blade geometry file into structured data
     * @param filePath Path to blade geometry file
     * @return Unique pointer to BladeGeometryData containing parsed sections
     * @throws std::runtime_error if file cannot be opened, parsing fails, or no data found
     * @note Processes lines starting with "DEF" as data rows, "#" as headers
     */
    std::unique_ptr<IStructuredData> parseFile(const std::string& filePath);

    /**
     * @brief Gets list of supported file extensions
     * @return Vector of supported extensions: .dat, .txt, .csv
     */
    std::vector<std::string> getSupportedExtensions() const;
};

