#include "AirfoilGeometryData.h"
#include <set>
#include "MathUtility.h"
#include "AirfoilPolarData.h"


void AirfoilGeometryData::setName(const std::string& n) { name = n; }



void AirfoilGeometryData::setRelativeThickness(double thickness) { relativeThickness = thickness; }



void AirfoilGeometryData::addHeader(const std::string& header) {
    headers.push_back(header);
}

void AirfoilGeometryData::addMarker(const std::string& type, int index) {
    markers.emplace_back(type, index);
}

void AirfoilGeometryData::addCoordinate(int idx, double x, double y, double z, bool isTop, bool isTE) {
    coordinates.emplace_back(idx, x, y, z, isTop, isTE);
}

const std::string& AirfoilGeometryData::getName() const { return name; }

double AirfoilGeometryData::getRelativeThickness() const { return relativeThickness; }

const std::vector<AirfoilMarker>& AirfoilGeometryData::getMarkers() const { return markers; }

const std::vector<AirfoilCoordinate>& AirfoilGeometryData::getCoordinates() const { return coordinates; }

const std::vector<std::string>& AirfoilGeometryData::getHeaders() const { return headers; }

std::string AirfoilGeometryData::getTypeName() const  { return "AirfoilGeometry"; }

size_t AirfoilGeometryData::getRowCount() const  { return coordinates.size(); }

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

std::pair<std::vector<AirfoilCoordinate>, std::vector<AirfoilCoordinate>> AirfoilGeometryData::separateTopBottom(const std::vector<AirfoilCoordinate>& nodes) {
    std::vector<AirfoilCoordinate> top, bottom;

    for (const auto& node : nodes) {
        if (node.isTopSurface) {
            top.push_back(node);
        }
        else {
            bottom.push_back(node);
        }
    }

    return { top, bottom };
}

std::vector<double> AirfoilGeometryData::getUniqueXCoordinates(const std::vector<AirfoilCoordinate>& surface1,
    const std::vector<AirfoilCoordinate>& surface2) {

    std::set<double> xSet;

    // Collect x-coordinates from both surfaces
    for (const auto& node : surface1) {
        xSet.insert(node.x);
    }
    for (const auto& node : surface2) {
        xSet.insert(node.y);
    }

    // Convert to sorted vector
    std::vector<double> xCoords(xSet.begin(), xSet.end());
    return xCoords;
}

// Helper function to interpolate a single surface
std::vector<AirfoilCoordinate> AirfoilGeometryData::interpolateSurface(
    const std::vector<AirfoilCoordinate>& surface1,
    const std::vector<AirfoilCoordinate>& surface2,
    const std::vector<double>& xCoords,
    double percent,
    bool isTopSurface) {
    // Extract x,y coordinates
    std::vector<double> x1, y1, x2, y2;
    for (const auto& node : surface1) {
        x1.push_back(node.x);
        y1.push_back(node.y);
    }
    for (const auto& node : surface2) {
        x2.push_back(node.x);
        y2.push_back(node.y);
    }

    // For top surface, reverse for proper interpolation direction
    if (isTopSurface) {
        std::reverse(x1.begin(), x1.end());
        std::reverse(y1.begin(), y1.end());
        std::reverse(x2.begin(), x2.end());
        std::reverse(y2.begin(), y2.end());
    }

    std::vector<AirfoilCoordinate> result;
    int indexCounter = 0;

    for (double x : xCoords) {
        // Interpolate y-values at this x-coordinate
        double y1_interp = MathUtility::linearInterpolation(x,x1, y1);
        double y2_interp = MathUtility::linearInterpolation(x,x2, y2);

        // Linear interpolation between the two surfaces
        double y_final = (1.0 - percent) * y2_interp + percent * y1_interp;

        // Create node (adjust x order for top surface)
        double x_final = isTopSurface ? xCoords[xCoords.size() - 1 - (indexCounter % xCoords.size())] : x;
        result.emplace_back(indexCounter++, x_final, y_final, 0,isTopSurface, false);
    }

    return result;
}

// Helper function to combine top and bottom surfaces
std::vector<AirfoilCoordinate> AirfoilGeometryData::combineTopBottomSurfaces(const std::vector<AirfoilCoordinate>& top,
    const std::vector<AirfoilCoordinate>& bottom) {
    std::vector<AirfoilCoordinate> combined;
    int indexCounter = 0;

    // Add leading edge point (1.0, 0.0)
    combined.emplace_back(indexCounter++, 1.0, 0.0, true, false);

    // Add top surface (excluding endpoints to avoid duplicates)
    for (size_t i = 0; i < top.size(); ++i) {
        if (top[i].x != 1.0 || top[i].y != 0.0) { // Skip if it's the endpoint
            combined.emplace_back(indexCounter++, top[i].x, top[i].y, 0, true, false);
        }
    }

    // Add bottom surface (excluding leading edge point)
    for (size_t i = 0; i < bottom.size(); ++i) {
        if (bottom[i].x != 0.0 || bottom[i].y != 0.0) { // Skip leading edge
            combined.emplace_back(indexCounter++, bottom[i].x, bottom[i].y, 0,false, false);
        }
    }

    // Add trailing edge point (1.0, 0.0)
    combined.emplace_back(indexCounter++, 1.0, 0.0, 0,false, true);

    return combined;
}

std::unique_ptr<AirfoilGeometryData> AirfoilGeometryData::interpolateBetweenGeometries(const AirfoilGeometryData& leftGeometry, const AirfoilGeometryData& rightGeometry, double targetThickness)
{
    // Determine which airfoil is thicker and which is thinner
    const AirfoilGeometryData* airfoilThick = nullptr;
    const AirfoilGeometryData* airfoilThin = nullptr;
    double thickThickness, thinThickness;

    if (leftGeometry.getRelativeThickness() > rightGeometry.getRelativeThickness()) {
        airfoilThick = &leftGeometry;
        airfoilThin = &rightGeometry;
        thickThickness = leftGeometry.getRelativeThickness();
        thinThickness = rightGeometry.getRelativeThickness();
    }
    else {
        airfoilThick = &rightGeometry;
        airfoilThin = &leftGeometry;
        thickThickness = rightGeometry.getRelativeThickness();
        thinThickness = leftGeometry.getRelativeThickness();
    }

    // Check if target thickness is within bounds
    if (targetThickness < thinThickness || targetThickness > thickThickness) {
        throw std::invalid_argument("Target thickness is outside the range of input geometries");
    }

    // Calculate interpolation factor (0 = thin airfoil, 1 = thick airfoil)
    double percent = (targetThickness - thinThickness) / (thickThickness - thinThickness);

    // Create result airfoil
    auto result = std::make_unique<AirfoilGeometryData>();
    result->setRelativeThickness(targetThickness);

    // Separate nodes into top and bottom surfaces
    auto [topThick, bottomThick] = separateTopBottom(airfoilThick->coordinates);
    auto [topThin, bottomThin] = separateTopBottom(airfoilThin->coordinates);

    // Get unique x-coordinates from both airfoils
    std::vector<double> xCoordsTop = getUniqueXCoordinates(topThick, topThin);
    std::vector<double> xCoordsBottom = getUniqueXCoordinates(bottomThick, bottomThin);

    // Interpolate top surface
    auto interpolatedTop = interpolateSurface(topThick, topThin, xCoordsTop, percent, true);

    // Interpolate bottom surface  
    auto interpolatedBottom = interpolateSurface(bottomThick, bottomThin, xCoordsBottom, percent, false);

    // Combine surfaces into result
    result->coordinates = combineTopBottomSurfaces(interpolatedTop, interpolatedBottom);

    return result;
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


