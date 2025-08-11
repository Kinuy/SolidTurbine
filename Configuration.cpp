#include "Configuration.h"

const BladeGeometryData* Configuration::getBladeGeometry(const std::string& key = "blade_geometry") const {
    return getStructuredData<BladeGeometryData>(key);
}