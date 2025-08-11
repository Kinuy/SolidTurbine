#pragma once
#include <unordered_map>
#include <string>
#include <any>
#include <stdexcept>
#include <memory>

#include "IStructuredData.h"
#include "BladeGeometryData.h"

// Configuration stores parsed values
class Configuration {
private:

    std::unordered_map<std::string, std::any> values;
    
    std::unordered_map<std::string, std::unique_ptr<IStructuredData>> structuredData;

public:
    void setValue(const std::string& key, std::any value) {
        values[key] = std::move(value);
    }

    void setStructuredData(const std::string& key, std::unique_ptr<IStructuredData> data) {
        structuredData[key] = std::move(data);
    }

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

    template<typename T>
    const T* getStructuredData(const std::string& key) const {
        auto it = structuredData.find(key);
        if (it == structuredData.end()) {
            return nullptr;
        }

        return dynamic_cast<const T*>(it->second.get());
    }

    bool hasValue(const std::string& key) const {
        return values.find(key) != values.end();
    }

    // Convenience methods
    double getDouble(const std::string& key) const { return getValue<double>(key); }

    int getInt(const std::string& key) const { return getValue<int>(key); }
    
    std::string getString(const std::string& key) const { return getValue<std::string>(key); }
    
    bool getBool(const std::string& key) const { return getValue<bool>(key); }
    
    std::string getFilePath(const std::string& key) const { return getValue<std::string>(key); }

    const BladeGeometryData* getBladeGeometry(const std::string& key) const;
};

