#include "Configuration.h"

const BladeGeometryData* Configuration::getBladeGeometry(const std::string& key) const {
    return getStructuredData<BladeGeometryData>(key);
}

const AirfoilPerformanceFileListData* Configuration::getAirfoilPerformanceFileList(const std::string& key) const {
    return getStructuredData<AirfoilPerformanceFileListData>(key);
}

const AirfoilGeometryFileListData* Configuration::getAirfoilGeometryFileList(const std::string& key) const {
    return getStructuredData<AirfoilGeometryFileListData>(key);
}


// Check if collection exists and has data
bool Configuration::hasCollection(const std::string& collectionKey) const {
    auto it = collectionData.find(collectionKey);
    return it != collectionData.end() && !it->second.empty();
}

// Check if structured data exists
bool Configuration::hasStructuredData(const std::string& key) const {
    return structuredData.find(key) != structuredData.end();
}

void Configuration::setValue(const std::string& key, std::any value) {
    values[key] = std::move(value);
}

void Configuration::setStructuredData(const std::string& key, std::unique_ptr<IStructuredData> data) {
    structuredData[key] = std::move(data);
}

// Store collections of structured data (e.g., multiple airfoil geometries)
void Configuration::addToCollection(const std::string& collectionKey, std::unique_ptr<IStructuredData> data) {
    collectionData[collectionKey].push_back(std::move(data));
}

bool Configuration::hasValue(const std::string& key) const {
    return values.find(key) != values.end();
}

double Configuration::getDouble(const std::string& key) const { 
    return getValue<double>(key); }

int Configuration::getInt(const std::string& key) const { 
    return getValue<int>(key); }

std::string Configuration::getString(const std::string& key) const { 
    return getValue<std::string>(key); }

bool Configuration::getBool(const std::string& key) const { 
    return getValue<bool>(key); }

std::string Configuration::getFilePath(const std::string& key) const { 
    return getValue<std::string>(key); }

std::vector<const AirfoilGeometryData*> Configuration::getAirfoilGeometries(const std::string& collectionKey) const {
    return getCollection<AirfoilGeometryData>(collectionKey);
}

const AirfoilGeometryData* Configuration::getAirfoilGeometryByName(const std::string& airfoilName, const std::string& collectionKey) const {
    auto geometries = getAirfoilGeometries(collectionKey);

    for (const auto* geom : geometries) {
        if (geom->getName().find(airfoilName) != std::string::npos) {
            return geom;
        }
    }

    throw std::runtime_error("Airfoil geometry not found for: " + airfoilName);
}