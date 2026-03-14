#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <algorithm>
#include <numbers>

#include "AirfoilPolarData.h"
#include "IDataFileParser.h"

/**
 * @brief File parser implementation for airfoil aerodynamic performance data
 *
 * AirfoilPerformanceParser implements IDataFileParser to read and parse airfoil
 * performance files containing aerodynamic coefficients (lift, drag, moment)
 * across angle of attack ranges. Supports two file formats, auto-detected from
 * the first content line:
 *
 * ## Format 1 — Native keyword format
 * Header keywords (REFNUM, XA, THICK, REYN, DEPANG, NALPHA, NVALS) followed
 * by numeric data rows (alpha [deg], cl, cd, cm).
 *
 * ## Format 2 — Tecplot ASCII (.pol / .dat)
 * Standard Tecplot point format:
 * ```
 * TITLE="Polarname.pol"
 * VARIABLES="alpha [deg]" "c_l [-]" "c_d [-]" "c_m [-]"
 * ZONE I=721, F=POINT, T="Polarname.pol"
 * AUXDATA renum="5000000"
 * AUXDATA ma="0.3"          (or "None" → defaults to 0.0)
 * AUXDATA airfoil="Name"
 * AUXDATA stallAngle="14.5"
 * ...
 * -1.8000E+02  -7.3089E-01  2.1069E-02  -3.3909E-01
 * ```
 * AUXDATA keys extracted: renum, ma, airfoil, stallAngle, negStallAngle,
 * designAngle, opt, tool, addons, runnum, ar, path.
 *
 * @see IDataFileParser for the base interface
 * @see AirfoilPolarData for output data structure
 * @see AirfoilPolarPoint for individual data points
 *
 * @example
 * ```cpp
 * AirfoilPerformanceParser parser;
 * auto data = parser.parseFile("naca0012.dat");       // native format
 * auto data2 = parser.parseFile("polar.pol");         // Tecplot format
 * ```
 */
class AirfoilPerformanceParser : public IDataFileParser
{

private:

    /**
     * @brief Supported file format variants
     */
    enum class PolarFormat {
        Native,  ///< Keyword-based format (REFNUM, THICK, REYN, ...)
        Tecplot  ///< Tecplot ASCII point format (TITLE=, ZONE, AUXDATA, ...)
    };

    /**
     * @brief Splits line into tokens using delimiter with automatic fallback
     * @param line Input line to tokenize
     * @param delimiter Primary delimiter character (default: tab)
     * @return Vector of trimmed, non-empty tokens
     * @note Falls back to space delimiter if tab produces ≤1 token
     */
    std::vector<std::string> tokenizeLine(const std::string &line, char delimiter = '\t') const;

    /**
     * @brief Checks if tokens represent a native-format header line
     * @param tokens Tokenized line to check
     * @return true if line contains header keywords (REFNUM, XA, THICK, etc.)
     */
    bool isHeaderLine(const std::vector<std::string> &tokens) const;

    /**
     * @brief Checks if tokens represent a numeric data line (alpha cl cd cm)
     * @param tokens Tokenized line to check
     * @return true if line has ≥4 tokens and first token is numeric
     */
    bool isDataLine(const std::vector<std::string> &tokens) const;

    /**
     * @brief Detects the format of an open performance file from its first content line
     * @param firstContentLine First non-empty, non-comment line already read from the file
     * @return Detected PolarFormat variant
     */
    PolarFormat detectFormat(const std::string& firstContentLine) const;

    /**
     * @brief Parses the native keyword format
     *
     * Processes REFNUM/XA/THICK/REYN/DEPANG/NALPHA/NVALS headers and
     * numeric alpha-cl-cd-cm data rows.
     *
     * @param file      Open file stream, positioned after @p firstLine
     * @param filePath  Path used for error messages
     * @param firstLine First content line already read (re-processed here)
     * @param perfData  Target AirfoilPolarData to populate
     */
    void parseNativeFormat(std::ifstream& file,
                           const std::string& filePath,
                           const std::string& firstLine,
                           AirfoilPolarData& perfData) const;

    /**
     * @brief Parses the Tecplot ASCII point format
     *
     * Extracts polar name and point count from ZONE line, Reynolds and Mach
     * numbers from AUXDATA lines, and numeric data rows (alpha already in
     * degrees, converted to radians on storage).
     *
     * @param file      Open file stream, positioned after @p firstLine
     * @param filePath  Path used for error messages
     * @param firstLine First content line already read (TITLE= line)
     * @param perfData  Target AirfoilPolarData to populate
     */
    void parseTecplotFormat(std::ifstream& file,
                            const std::string& filePath,
                            const std::string& firstLine,
                            AirfoilPolarData& perfData) const;

    /**
     * @brief Extracts the value from a Tecplot AUXDATA line
     *
     * Parses lines of the form `AUXDATA key="value"` and returns the
     * unquoted value string.
     *
     * @param line Full AUXDATA line
     * @param key  AUXDATA key to extract (case-insensitive)
     * @param value Output value string (unchanged if key not found)
     * @return true if the key was found and value was extracted
     */
    bool extractAuxDataValue(const std::string& line,
                             const std::string& key,
                             std::string& value) const;

public:
    /**
     * @brief Parses an airfoil performance file into structured data
     *
     * Auto-detects the file format from the first content line:
     * lines starting with `TITLE=` trigger Tecplot parsing; all other
     * content triggers native keyword parsing.
     *
     * @param filePath Path to airfoil performance file
     * @return Unique pointer to AirfoilPolarData with parsed coefficients
     * @throws std::runtime_error if file cannot be opened, parsing fails, or no data found
     */
    std::unique_ptr<IStructuredData> parseFile(const std::string &filePath) override;

    /**
     * @brief Gets list of supported file extensions
     * @return Vector of supported extensions: .dat, .txt, .perf, .aero, .pol
     */
    std::vector<std::string> getSupportedExtensions() const override;
};

