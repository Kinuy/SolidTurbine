#pragma once
#include <unordered_map>
#include <string>
#include <any>
#include <stdexcept>
#include <memory>

#include "IStructuredData.h"
#include "BladeGeometryData.h"
#include "AirfoilPerformanceFileListData.h"
#include "AirfoilGeometryFileListData.h"
#include "AirfoilGeometryData.h"
#include "AirfoilPerformanceData.h"


// Configuration stores parsed values
class Configuration {

private:

    std::unordered_map<std::string, std::any> values;
    
    std::unordered_map<std::string, std::unique_ptr<IStructuredData>> structuredData;

    std::unordered_map<std::string, std::vector<std::unique_ptr<IStructuredData>>> collectionData; // For collections of files

public:

    void setValue(const std::string& key, std::any value);

    void setStructuredData(const std::string& key, std::unique_ptr<IStructuredData> data);

    // Store collections of structured data (e.g., multiple airfoil geometries)
    void addToCollection(const std::string& collectionKey, std::unique_ptr<IStructuredData> data);

	// Get a value by key with type safety
    template<typename T>
    T getValue(const std::string& key) const {
        auto it = values.find(key);
        if (it == values.end()) {
            throw std::runtime_error("Configuration key '" + key + "' not found");
        }

        try {
            return std::any_cast<T>(it->second);
        }
        catch (const std::bad_any_cast&) {
            throw std::runtime_error("Configuration key '" + key + "' has wrong type");
        }
    }

	// Get structured data by key
    template<typename T>
    const T* getStructuredData(const std::string& key) const {
        auto it = structuredData.find(key);
        if (it == structuredData.end()) {
            return nullptr;
        }

        return dynamic_cast<const T*>(it->second.get());
    }

    // Get collection of structured data
    template<typename T>
    std::vector<const T*> getCollection(const std::string& collectionKey) const {
        std::vector<const T*> result;
        auto it = collectionData.find(collectionKey);
        if (it != collectionData.end()) {
            for (const auto& data : it->second) {
                const T* typedData = dynamic_cast<const T*>(data.get());
                if (typedData) {
                    result.push_back(typedData);
                }
            }
        }
        return result;
    }

    bool hasValue(const std::string& key) const;

    // Check if collection exists and has data
    bool hasCollection(const std::string& collectionKey) const;

	// Check if structured data exists
    bool hasStructuredData(const std::string& key) const;

    // Convenience methods
    double getDouble(const std::string& key) const;

    int getInt(const std::string& key) const;
    
    std::string getString(const std::string& key) const;
    
    bool getBool(const std::string& key) const;
    
    std::string getFilePath(const std::string& key) const;

    const BladeGeometryData* getBladeGeometry(const std::string& key = "blade_geometry") const;

    const AirfoilPerformanceFileListData* getAirfoilPerformanceFileList(const std::string& key = "airfoil_performance_files") const;

    const AirfoilGeometryFileListData* getAirfoilGeometryFileList(const std::string& key = "airfoil_geometry_files") const;

    // Individual airfoil geometries collection convenience method
    std::vector<const AirfoilGeometryData*> getAirfoilGeometries(const std::string& collectionKey = "loaded_airfoil_geometries") const;

    // Find specific airfoil geometry by name
    const AirfoilGeometryData* getAirfoilGeometryByName(const std::string& airfoilName, const std::string& collectionKey = "loaded_airfoil_geometries") const;

    // Individual airfoil performance collection convenience method
    std::vector<const AirfoilPerformanceData*> getAirfoilPerformances(const std::string& collectionKey = "loaded_airfoil_performances") const;

    // Find specific airfoil performance by reference number
    const AirfoilPerformanceData* getAirfoilPerformanceByRefNum(const std::string& refNum, const std::string& collectionKey = "loaded_airfoil_performances") const;

    // Find airfoil performance by thickness and Reynolds number
    const AirfoilPerformanceData* getAirfoilPerformanceByConditions(double thickness, double reynolds,
        double toleranceThickness = 5.0,
        double toleranceReynolds = 500000.0,
        const std::string& collectionKey = "loaded_airfoil_performances") const;

};

