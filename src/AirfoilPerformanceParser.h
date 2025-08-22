#pragma once

#include<vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <algorithm>

#include "AirfoilPolarData.h"
#include "IDataFileParser.h"


/**
 * @brief File parser implementation for airfoil aerodynamic performance data
 *
 * AirfoilPerformanceParser implements IDataFileParser to read and parse airfoil
 * performance files containing aerodynamic coefficients (lift, drag, moment)
 * across angle of attack ranges. Handles both metadata headers and numeric
 * data with flexible formatting and validation.
 *
 * ## Supported File Format
 * - **Header Lines**: Metadata with keywords (REFNUM, XA, THICK, REYN, etc.)
 * - **Data Lines**: Numeric values for alpha, cl, cd, cm coefficients
 * - **Comments**: Lines starting with '#' (preserved as headers)
 * - **Delimiters**: Tab-separated preferred, space-separated fallback
 * - **Extensions**: .dat, .txt, .perf, .aero files
 *
 * ## Key Features
 * - Automatic delimiter detection and fallback
 * - Metadata extraction from header lines
 * - Data consistency validation (NALPHA vs actual rows)
 * - Flexible coefficient parsing (cm optional)
 *
 * ## Typical File Structure
 * ```
 * # Airfoil performance data
 * REFNUM NACA0012
 * THICK 12.0
 * REYN 1000000
 * NALPHA 37
 * -10.0  -0.8   0.05  0.1
 *   0.0   0.0   0.008 0.0
 *  10.0   0.8   0.02 -0.1
 * ```
 *
 * @see IDataFileParser for the base interface
 * @see AirfoilPerformanceData for output data structure
 * @see AirfoilPerformancePoint for individual data points
 *
 * @example
 * ```cpp
 * AirfoilPerformanceParser parser;
 * auto data = parser.parseFile("naca0012.dat");
 * auto perfData = static_cast<AirfoilPerformanceData*>(data.get());
 * ```
 */
class AirfoilPerformanceParser : public IDataFileParser {

private:

    /**
     * @brief Splits line into tokens using delimiter with automatic fallback
     * @param line Input line to tokenize
     * @param delimiter Primary delimiter character (default: tab)
     * @return Vector of trimmed, non-empty tokens
     * @note Falls back to space delimiter if tab produces ≤1 token
     */
    std::vector<std::string> tokenizeLine(const std::string& line, char delimiter = '\t') const;

    /**
     * @brief Checks if tokens represent a header line with metadata
     * @param tokens Tokenized line to check
     * @return true if line contains header keywords (REFNUM, XA, THICK, etc.)
     */
    bool isHeaderLine(const std::vector<std::string>& tokens) const;

    /**
     * @brief Checks if tokens represent a data line with numeric values
     * @param tokens Tokenized line to check
     * @return true if line has ≥4 tokens and first token is numeric (alpha value)
     */
    bool isDataLine(const std::vector<std::string>& tokens) const;

public:

    /**
     * @brief Parses airfoil performance file into structured data
     * @param filePath Path to airfoil performance file
     * @return Unique pointer to AirfoilPerformanceData with parsed coefficients
     * @throws std::runtime_error if file cannot be opened, parsing fails, or no data found
     * @note Validates data consistency and warns about NALPHA mismatches
     */
    std::unique_ptr<IStructuredData> parseFile(const std::string& filePath) override;

    /**
     * @brief Gets list of supported file extensions
     * @return Vector of supported extensions: .dat, .txt, .perf, .aero
     */
    std::vector<std::string> getSupportedExtensions() const override;

};

