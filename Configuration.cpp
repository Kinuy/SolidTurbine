#include "Configuration.h"

const BladeGeometryData* Configuration::getBladeGeometry(const std::string& key = "blade_geometry") const {
    return getStructuredData<BladeGeometryData>(key);
}

const AirfoilPerformanceFileListData* Configuration::getAirfoilPerformanceFileList(const std::string& key = "airfoil_performance_files") const {
    return getStructuredData<AirfoilPerformanceFileListData>(key);
}

const AirfoilGeometryFileListData* Configuration::getAirfoilGeometryFileList(const std::string& key = "airfoil_geometry_files") const {
    return getStructuredData<AirfoilGeometryFileListData>(key);
}