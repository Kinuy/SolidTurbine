#pragma once
#include <unordered_map>
#include <string>
#include <any>
#include <stdexcept>
#include <memory>
#include <iostream>

#include "IStructuredData.h"
#include "BladeGeometryData.h"
#include "AirfoilPerformanceFileListData.h"
#include "AirfoilGeometryFileListData.h"
#include "AirfoilGeometryData.h"
#include "AirfoilPolarData.h"
#include "BladeInterpolator.h"
#include "HorizontalTurbine.h"

/**
 * @brief Configuration container for parsed values and structured data
 *
 * Configuration provides centralized storage and type-safe access to parsed
 * configuration parameters, structured data objects, and collections. Supports
 * both simple values (primitives) and complex data structures through a
 * unified interface.
 *
 * ## Key Features
 * - **Type Safety**: Template-based value retrieval with runtime checking
 * - **Mixed Storage**: Simple values, structured data, and collections
 * - **Dynamic Typing**: std::any for flexible value storage
 * - **Collection Support**: Groups of related structured data items
 *
 * ## Storage Categories
 * - **Values**: Primitives (double, int, string, bool) in std::any containers
 * - **Structured Data**: Complex objects implementing IStructuredData
 * - **Collections**: Arrays of structured data for related items
 *
 * @see IStructuredData for complex data object interface
 * @see ConfigurationParser for configuration file processing
 *
 * @example
 * ```cpp
 * Configuration config;
 * config.setValue("speed", 25.0);
 * config.setStructuredData("blade", std::move(bladeData));
 *
 * double speed = config.getDouble("speed");
 * const BladeGeometryData* blade = config.getBladeGeometry("blade");
 * ```
 */
class Configuration
{

private:
    /**
     * @brief Storage for simple configuration values of any type
     *
     * Maps parameter names to std::any containers holding parsed values.
     * Supports all fundamental types through template-based access.
     */
    std::unordered_map<std::string, std::any> values;

    /**
     * @brief Storage for individual structured data objects
     *
     * Maps keys to unique pointers of structured data implementing
     * IStructuredData interface (e.g., blade geometry, airfoil data).
     */
    std::unordered_map<std::string, std::unique_ptr<IStructuredData>> structuredData;

    /**
     * @brief Storage for collections of related structured data
     *
     * Maps collection keys to vectors of structured data objects.
     * Used for handling multiple files of the same type (e.g., multiple airfoils).
     */
    std::unordered_map<std::string, std::vector<std::unique_ptr<IStructuredData>>> collectionData;

public:
    /**
     * @brief Sets a configuration value
     * @param key Parameter identifier
     * @param value Value to store (takes ownership)
     */
    void setValue(const std::string &key, std::any value);

    /**
     * @brief Sets structured data for key
     * @param key Data identifier
     * @param data Structured data to store (takes ownership)
     */
    void setStructuredData(const std::string &key, std::unique_ptr<IStructuredData> data);

    /**
     * @brief Adds data to a collection
     * @param collectionKey Collection identifier
     * @param data Data to add (takes ownership)
     */
    void addToCollection(const std::string &collectionKey, std::unique_ptr<IStructuredData> data);

    /**
     * @brief Gets a configuration value by key with type safety
     *
     * Retrieves a stored value and performs safe type conversion using std::any_cast.
     * Provides compile-time type specification and runtime type verification.
     *
     * @tparam T Expected type of the stored value
     * @param key Configuration parameter identifier
     * @return Value of type T
     * @throws std::runtime_error if key not found
     * @throws std::runtime_error if stored type doesn't match requested type T
     *
     * @example
     * ```cpp
     * double speed = config.getValue<double>("max_speed");
     * std::string name = config.getValue<std::string>("project_name");
     * ```
     */
    template <typename T>
    T getValue(const std::string &key) const
    {
        auto it = values.find(key);
        if (it == values.end())
        {
            throw std::runtime_error("Configuration key '" + key + "' not found");
        }

        try
        {
            return std::any_cast<T>(it->second);
        }
        catch (const std::bad_any_cast &)
        {
            throw std::runtime_error("Configuration key '" + key + "' has wrong type");
        }
    }

    /**
     * @brief Gets structured data by key with type casting
     *
     * Retrieves structured data and performs dynamic cast to the requested type.
     * Returns nullptr if key doesn't exist or cast fails.
     *
     * @tparam T Expected derived type of IStructuredData
     * @param key Data identifier
     * @return Pointer to T or nullptr if not found/wrong type
     *
     * @example
     * ```cpp
     * const BladeGeometryData* blade = config.getStructuredData<BladeGeometryData>("blade");
     * if (blade) {  use blade data }
     **/
    template <typename T>
    const T *getStructuredData(const std::string &key) const
    {
        auto it = structuredData.find(key);
        if (it == structuredData.end())
        {
            return nullptr;
        }

        return dynamic_cast<const T *>(it->second.get());
    }

    /**
     * @brief Gets collection of structured data with type filtering
     *
     * Retrieves all items from a collection that can be cast to type T.
     * Items that don't match type T are silently filtered out.
     *
     * @tparam T Expected derived type of IStructuredData
     * @param collectionKey Collection identifier
     * @return Vector of pointers to T (may be empty)
     *
     * @example
     * ```cpp
     * auto airfoils = config.getCollection<AirfoilGeometryData>("airfoils");
     * for (const auto* airfoil : airfoils) {  process airfoil }
     **/
    template <typename T>
    std::vector<const T *> getCollection(const std::string &collectionKey) const
    {
        std::vector<const T *> result;
        auto it = collectionData.find(collectionKey);
        if (it != collectionData.end())
        {
            for (const auto &data : it->second)
            {
                const T *typedData = dynamic_cast<const T *>(data.get());
                if (typedData)
                {
                    result.push_back(typedData);
                }
            }
        }
        return result;
    }

    /**
     * @brief Checks if value exists for key
     * @param key Parameter identifier
     * @return true if value exists
     */
    bool hasValue(const std::string &key) const;

    /**
     * @brief Checks if collection exists and contains data
     * @param collectionKey Collection identifier
     * @return true if collection exists and is non-empty
     */
    bool hasCollection(const std::string &collectionKey) const;

    /**
     * @brief Checks if structured data exists for key
     * @param key Data identifier
     * @return true if structured data exists
     */
    bool hasStructuredData(const std::string &key) const;

    /**
     * @brief Gets double value by key
     * @param key Parameter identifier
     * @return Double value
     */
    double getDouble(const std::string &key) const;

    /**
     * @brief Gets integer value by key
     * @param key Parameter identifier
     * @return Integer value
     */
    int getInt(const std::string &key) const;

    /**
     * @brief Gets string value by key
     * @param key Parameter identifier
     * @return String value
     */
    std::string getString(const std::string &key) const;

    /**
     * @brief Gets boolean value by key
     * @param key Parameter identifier
     * @return Boolean value
     */
    bool getBool(const std::string &key) const;

    /**
     * @brief Gets file path value by key
     * @param key Parameter identifier
     * @return File path string
     */
    std::string getFilePath(const std::string &key) const;

    /**
     * @brief Gets blade geometry data by key
     * @param key Parameter key for blade geometry
     * @return Pointer to BladeGeometryData or nullptr if not found
     */
    const BladeGeometryData *getBladeGeometry(const std::string &key = "blade_geometry") const;

    /**
     * @brief Gets airfoil performance file list by key
     * @param key Parameter key for file list
     * @return Pointer to AirfoilPerformanceFileListData or nullptr if not found
     */
    const AirfoilPerformanceFileListData *getAirfoilPerformanceFileList(const std::string &key = "airfoil_performance_files") const;

    /**
     * @brief Gets airfoil geometry file list by key
     * @param key Parameter key for file list
     * @return Pointer to AirfoilGeometryFileListData or nullptr if not found
     */
    const AirfoilGeometryFileListData *getAirfoilGeometryFileList(const std::string &key = "airfoil_geometry_files") const;

    /**
     * @brief Gets all airfoil geometries from collection
     * @param collectionKey Collection identifier
     * @return Vector of airfoil geometry pointers
     */
    std::vector<const AirfoilGeometryData *> getAirfoilGeometries(const std::string &collectionKey = "loaded_airfoil_geometries") const;

    /**
     * @brief Finds airfoil geometry by name
     * @param airfoilName Name to search for
     * @param collectionKey Collection identifier
     * @return Pointer to matching airfoil geometry
     * @throws std::runtime_error if airfoil not found
     */
    const AirfoilGeometryData *getAirfoilGeometryByName(const std::string &airfoilName, const std::string &collectionKey = "loaded_airfoil_geometries") const;

    /**
     * @brief Gets all airfoil performance data from collection
     * @param collectionKey Collection identifier
     * @return Vector of airfoil performance pointers
     */
    std::vector<const AirfoilPolarData *> getAirfoilPerformances(const std::string &collectionKey = "loaded_airfoil_performances") const;

    /**
     * @brief Finds airfoil performance by reference number
     * @param refNum Reference number to search for
     * @param collectionKey Collection identifier
     * @return Pointer to matching airfoil performance
     * @throws std::runtime_error if performance data not found
     */
    const AirfoilPolarData *getAirfoilPerformanceByRefNum(const std::string &refNum, const std::string &collectionKey = "loaded_airfoil_performances") const;

    /**
     * @brief Finds airfoil performance by thickness and Reynolds number
     * @param thickness Target relative thickness [%]
     * @param reynolds Target Reynolds number
     * @param toleranceThickness Thickness tolerance [%]
     * @param toleranceReynolds Reynolds number tolerance
     * @param collectionKey Collection identifier
     * @return Pointer to matching airfoil performance
     * @throws std::runtime_error if no matching data found
     */
    const AirfoilPolarData *getAirfoilPerformanceByConditions(double thickness, double reynolds,
                                                              double toleranceThickness = 5.0,
                                                              double toleranceReynolds = 500000.0,
                                                              const std::string &collectionKey = "loaded_airfoil_performances") const;

    /**
     * @brief Creates blade geometry interpolator with specified method
     * @param method Interpolation method enum
     * @return Unique pointer to configured interpolator
     * @throws std::runtime_error if required data not available
     */
    std::unique_ptr<BladeInterpolator> createBladeInterpolator(InterpolationMethod method = InterpolationMethod::LINEAR) const;

    /**
     * @brief Creates blade geometry interpolator from method name string
     * @param methodName Method name ("linear", "cubic", "akima", "monotonic")
     * @return Unique pointer to configured interpolator
     * @note Falls back to linear interpolation for unknown method names
     */
    std::unique_ptr<BladeInterpolator> createBladeInterpolator(const std::string &methodName) const;

    /**
     * @brief Creates turbine with specified method
     * @return Unique pointer to configured turbine
     */
    std::unique_ptr<HorizontalTurbine> getTurbine() const;
};
