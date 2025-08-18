#pragma once

#include <string>

/**
 * @brief Base interface for structured data containers
 *
 * IStructuredData provides a common interface for data objects that contain
 * structured information like tables, datasets, or collections. Enables
 * polymorphic handling of different data types through shared interface.
 *
 * ## Key Features
 * - **Type Identification**: Named data types for runtime identification
 * - **Size Information**: Row count for data validation and iteration
 * - **Polymorphic Storage**: Common interface for diverse data structures
 * - **Memory Management**: Virtual destructor for proper cleanup
 *
 * ## Common Implementations
 * - **BladeGeometryData**: Blade section geometric parameters
 * - **AirfoilGeometryData**: Airfoil coordinate data
 * - **AirfoilPerformanceData**: Performance curves and coefficients
 * - **FileListData**: Collections of file references
 *
 * @see BladeGeometryData, AirfoilGeometryData for concrete implementations
 * @see Configuration for storage and retrieval of structured data
 *
 * @example
 * ```cpp
 * std::unique_ptr<IStructuredData> data = parser->parseFile("blade.dat");
 * std::string type = data->getTypeName();  // "BladeGeometry"
 * size_t rows = data->getRowCount();       // Number of sections
 * ```
 */
class IStructuredData {

public:

    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~IStructuredData() = default;

    /**
     * @brief Gets the type name identifier for this data structure
     * @return String identifying the data type (e.g., "BladeGeometry", "AirfoilPerformance")
     */
    virtual std::string getTypeName() const = 0;

    /**
     * @brief Gets the number of data rows/records in this structure
     * @return Number of rows/entries in the data collection
     */
    virtual size_t getRowCount() const = 0;

};