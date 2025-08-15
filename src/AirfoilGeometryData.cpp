#include "AirfoilGeometryData.h"


void AirfoilGeometryData::setName(const std::string& n) { name = n; }

void AirfoilGeometryData::setRelativeThickness(double thickness) { relativeThickness = thickness; }

void AirfoilGeometryData::addHeader(const std::string& header) {
    headers.push_back(header);
}

void AirfoilGeometryData::addMarker(const std::string& type, int index) {
    markers.emplace_back(type, index);
}

void AirfoilGeometryData::addCoordinate(double x, double y) {
    coordinates.emplace_back(x, y);
}

// Getters
const std::string& AirfoilGeometryData::getName() const { return name; }

double AirfoilGeometryData::getRelativeThickness() const { return relativeThickness; }

const std::vector<AirfoilMarker>& AirfoilGeometryData::getMarkers() const { return markers; }

const std::vector<AirfoilCoordinate>& AirfoilGeometryData::getCoordinates() const { return coordinates; }

const std::vector<std::string>& AirfoilGeometryData::getHeaders() const { return headers; }

std::string AirfoilGeometryData::getTypeName() const  { return "AirfoilGeometry"; }

size_t AirfoilGeometryData::getRowCount() const  { return coordinates.size(); }

// Convenience methods for airfoil analysis
AirfoilCoordinate AirfoilGeometryData::getLeadingEdge() const {
    auto marker = getMarkerByType("LE");
    if (marker.index >= 0 && marker.index < static_cast<int>(coordinates.size())) {
        return coordinates[marker.index];
    }
    throw std::runtime_error("Leading edge marker not found");
}

AirfoilCoordinate AirfoilGeometryData::getTrailingEdge() const {
    auto marker = getMarkerByType("TE");
    if (marker.index >= 0 && marker.index < static_cast<int>(coordinates.size())) {
        return coordinates[marker.index];
    }
    throw std::runtime_error("Trailing edge marker not found");
}

std::vector<AirfoilCoordinate> AirfoilGeometryData::getUpperSurface() const {
    std::vector<AirfoilCoordinate> upperSurface;
    // Implementation would extract upper surface coordinates
    // This is a simplified version - you'd use markers to identify sections
    for (const auto& coord : coordinates) {
        if (coord.y >= 0) {  // Simplified: positive Y is upper surface
            upperSurface.push_back(coord);
        }
    }
    return upperSurface;
}

std::vector<AirfoilCoordinate> AirfoilGeometryData::getLowerSurface() const {
    std::vector<AirfoilCoordinate> lowerSurface;
    for (const auto& coord : coordinates) {
        if (coord.y < 0) {  // Simplified: negative Y is lower surface
            lowerSurface.push_back(coord);
        }
    }
    return lowerSurface;
}

double AirfoilGeometryData::getChordLength() const {
    auto le = getLeadingEdge();
    auto te = getTrailingEdge();
    return std::abs(te.x - le.x);
}

double AirfoilGeometryData::getMaxThickness() const {
    double maxThickness = 0.0;
    auto upperSurface = getUpperSurface();
    auto lowerSurface = getLowerSurface();

    // Find maximum thickness (simplified calculation)
    for (const auto& upper : upperSurface) {
        for (const auto& lower : lowerSurface) {
            if (std::abs(upper.x - lower.x) < 0.001) {  // Same x position
                double thickness = std::abs(upper.y - lower.y);
                maxThickness = std::max(maxThickness, thickness);
            }
        }
    }
    return maxThickness;
}

AirfoilMarker AirfoilGeometryData::getMarkerByType(const std::string& type) const {
    auto it = std::find_if(markers.begin(), markers.end(),
        [&type](const AirfoilMarker& marker) {
            return marker.type == type;
        });

    if (it == markers.end()) {
        throw std::runtime_error("Marker type not found: " + type);
    }
    return *it;
}