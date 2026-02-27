#pragma once

#include<vector>
#include<stdexcept>

#include "BladeGeometrySection.h"
#include "IStructuredData.h"

/**
 * @brief Structured data container for blade geometry information
 *
 * BladeGeometryData implements IStructuredData to store and manage blade
 * geometry sections along radial stations. Provides efficient storage and
 * retrieval of geometric data for turbomachinery blade analysis.
 *
 * ## Key Features
 * - **Radial Organization**: Data organized by blade radius values
 * - **Section-Based**: Each row represents a complete blade cross-section
 * - **Header Support**: Column descriptions for data interpretation
 * - **Flexible Access**: Retrieval by radius (with tolerance) or array index
 *
 * ## Typical Data Structure
 * Each BladeGeometrySection contains geometric parameters like chord length,
 * twist angle, thickness, and other blade properties at a specific radius.
 *
 * ## Use Cases
 * - CFD preprocessing and blade mesh generation
 * - Aerodynamic analysis and performance calculations
 * - Blade manufacturing and quality control
 * - Design optimization and parametric studies
 *
 * @see IStructuredData for the base interface
 * @see BladeGeometrySection for individual section data structure
 *
 * @example
 * ```cpp
 * BladeGeometryData blade;
 * blade.addHeader("Radius"); blade.addHeader("Chord");
 * blade.addRow(BladeGeometrySection{0.5, 0.12, 25.0}); // radius, chord, twist
 *
 * auto section = blade.getRowByRadius(0.5);
 * std::vector<double> radii = blade.getRadiusValues();
 * ```
 */
class BladeGeometryData : public IStructuredData {

private:

    /**
     * @brief Collection of blade geometry sections at different radial stations
     *
     * Each BladeGeometrySection contains complete geometric data for one
     * radial station along the blade span.
     */
    std::vector<BladeGeometrySection> rows;

    /**
     * @brief Column header names describing the geometric parameters
     *
     * Headers provide human-readable descriptions of the data fields
     * contained in each BladeGeometrySection.
     */
    std::vector<std::string> headers;

public:

    /**
     * @brief Adds a column header to the blade geometry data
     * @param header Header name/description to add
     */
    void addHeader(const std::string& header);

    /**
     * @brief Adds a blade geometry section to the data collection
     * @param row BladeGeometrySection containing radial station data
     */
    // The && is needed because to is bind to rvalue in parsing process
    // Normal copy is not possible due to unique pointers in BladeGeometrySection 
    void addRow(BladeGeometrySection&& row);

    /**
     * @brief Gets all blade geometry sections
     * @return Const reference to vector of BladeGeometrySection objects
     */
    const std::vector<BladeGeometrySection>& getRows() const;

    /**
     * @brief Gets all column headers
     * @return Const reference to vector of header strings
     */
    const std::vector<std::string>& getHeaders() const;

    /**
     * @brief Gets the data type identifier
     * @return Always returns "BladeGeometry"
     */
    std::string getTypeName() const;

    /**
    * @brief Gets the number of geometry sections
    * @return Number of rows in the data collection
    */
    size_t getRowCount() const;

    /**
     * @brief Finds blade geometry section by radius value with tolerance
     * @param radius Target radius value to search for
     * @param tolerance Acceptable difference from target (default: 0.001)
     * @return BladeGeometrySection at the specified radius
     * @throws std::runtime_error if no matching radius found within tolerance
     */
    BladeGeometrySection& getRowByRadius(double radius, double tolerance = 0.001);

    /**
     * @brief Finds blade geometry section by relative thickness with tolerance
     * @param relativeThickness Target relative thickness value to search for
     * @param tolerance Acceptable difference from target (default: 0.001)
     * @return BladeGeometrySection at the specified relative thickness
     * @throws std::runtime_error if no matching thickness found within tolerance
	 */
    BladeGeometrySection& getRowByRelativeThickness(double relativeThickness, double tolerance);

    /**
     * @brief Gets blade geometry section by array index
     * @param index Zero-based index into the rows vector
     * @return BladeGeometrySection at the specified index
     * @throws std::out_of_range if index is invalid
     */
    BladeGeometrySection& getRowByIndex(int index);

    /**
     * @brief Extracts all radius values from the geometry sections
     * @return Vector containing blade radius values from all rows
     */
    std::vector<double> getRadiusValues() const;

};
