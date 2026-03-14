#pragma once

#include "IDataFileParser.h"
#include "IStructuredData.h"
#include "BladeGeometryData.h"

#include <sstream>
#include <fstream>

/**
 * @brief File parser for blade geometry data files — standard and E format
 *
 * BladeGeometryParser implements IDataFileParser to read and parse blade
 * geometry files. It automatically detects the file format from the first
 * non-blank, non-comment line:
 *
 * - **Standard format**: first token is "DEF"; 10 columns per data row
 *   (bladeRadius, chord, twist°, relThick%, xt4, yt4, pcbaX, pcbaY,
 *   relativeTwistAxis, airfoilName)
 *
 * - **E format** (BladeGeometry_E): first token is "VERSIO"; tab-delimited,
 *   18 columns per data row (sectionIndex discarded, then bladeRadius … addOns,
 *   airfoilName). Includes VERSIO and FILE header lines.
 *
 * Both formats produce a `BladeGeometryData` object.  Fields not present in
 * the loaded format are left at their zero-initialised defaults in
 * `BladeGeometrySection`.
 *
 * ## Key Features
 * - Automatic format detection (VERSIO header → E-format path)
 * - Automatic delimiter detection (tab → space fallback) for standard format
 * - Whitespace trimming and empty-line skipping
 * - Line-by-line error reporting with line numbers
 *
 * @see IDataFileParser for the base interface
 * @see BladeGeometryData for output data structure
 * @see BladeGeometrySection for individual section data
 *
 * @example
 * ```cpp
 * BladeGeometryParser parser;
 * auto data = parser.parseFile("blade.dat");          // standard format
 * auto data2 = parser.parseFile("blade_E.dat");       // E-format, same call
 * auto bladeData = static_cast<BladeGeometryData*>(data.get());
 * ```
 */
class BladeGeometryParser : public IDataFileParser {

private:

    /**
     * @brief Splits a line into tokens using delimiter with automatic fallback
     * @param line   Input line to tokenize
     * @param delimiter Primary delimiter character (default: tab)
     * @return Vector of trimmed, non-empty tokens
     * @note Automatically falls back to space delimiter if tab produces ≤1 token
     */
    std::vector<std::string> tokenizeLine(const std::string& line, char delimiter = '\t') const;

    /**
     * @brief Parses the standard (DEF-prefixed) blade geometry format
     *
     * Called by parseFile() when the first content line does NOT start with
     * "VERSIO".  The already-read @p firstLine is processed as the first
     * data/header row so no line is skipped.
     *
     * @param stream    Open file stream positioned after @p firstLine
     * @param filePath  Original file path (for error messages)
     * @param data      Target BladeGeometryData to populate
     * @param firstLine First non-blank, non-comment line already read
     */
    void parseStandardFormat(std::ifstream& stream,
                             const std::string& filePath,
                             BladeGeometryData& data,
                             const std::string& firstLine) const;

    /**
     * @brief Parses the extended (VERSIO-prefixed) BladeGeometry_E format
     *
     * Called by parseFile() after the VERSIO line has been consumed and the
     * version string stored.  Reads FILE header and column-header line, then
     * parses 18-column tab-delimited data rows.
     *
     * @param stream   Open file stream positioned just after the VERSIO line
     * @param filePath Original file path (for error messages)
     * @param data     Target BladeGeometryData to populate
     */
    void parseEFormat(std::ifstream& stream,
                      const std::string& filePath,
                      BladeGeometryData& data) const;

public:

    /**
     * @brief Parses a blade geometry file into structured data
     *
     * Opens the file, reads the first non-blank line to detect the format,
     * then delegates to parseStandardFormat() or parseEFormat() accordingly.
     *
     * @param filePath Path to blade geometry file
     * @return Unique pointer to BladeGeometryData containing parsed sections
     * @throws std::runtime_error if file cannot be opened, parsing fails, or no data found
     */
    std::unique_ptr<IStructuredData> parseFile(const std::string& filePath) override;

    /**
     * @brief Gets list of supported file extensions
     * @return Vector containing: .dat, .txt, .csv
     */
    std::vector<std::string> getSupportedExtensions() const override;
};

