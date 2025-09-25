#include "AirfoilGeometryData.h"
#include <set>
#include "MathUtility.h"
#include "AirfoilPolarData.h"
#include <algorithm>
#include <iostream>


void AirfoilGeometryData::setName(const std::string& n) { name = n; }



void AirfoilGeometryData::setRelativeThickness(double thickness) { relativeThickness = thickness; }

void AirfoilGeometryData::setZCoordinates(double z)
{
    for (auto& coord : coordinates) {
        coord.z = z;
    }
}

void AirfoilGeometryData::addHeader(const std::string& header) {
    headers.push_back(header);
}

void AirfoilGeometryData::addMarker(const std::string& type, int index) {
    markers.emplace_back(type, index);
}

void AirfoilGeometryData::addCoordinate(int idx, double x, double y, double z, bool isTop, bool isTE, bool isTETE, bool isTEBE) {
    coordinates.emplace_back(idx, x, y, z, isTop, isTE, isTETE, isTEBE);
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

bool AirfoilGeometryData::coordinateIsTop(const double y) const
{
    if (y >= 0) {
        return true;
    }
    return false;
}

bool AirfoilGeometryData::coordinateIsTE(const double x) const
{
    if (x == 1) {
		return true;
    }
    return false;
}

void AirfoilGeometryData::findAndAssignTETEAndTEBEPoints()
{
    // Lambda to find both TETE and TEBE in single pass
    auto findTrailingEdgePoints = [](auto& coordinates) {
        auto teteIt = coordinates.end();
        auto tebeIt = coordinates.end();
        double maxXUpper = std::numeric_limits<double>::lowest();
        double maxXLower = std::numeric_limits<double>::lowest();

        for (auto it = coordinates.begin(); it != coordinates.end(); ++it) {
            if (it->y >= 0.0 && it->x > maxXUpper) {
                maxXUpper = it->x;
                teteIt = it;
            }
            else if (it->y < 0.0 && it->x > maxXLower) {
                maxXLower = it->x;
                tebeIt = it;
            }
        }

        return std::make_pair(teteIt, tebeIt);
        };

    // Reset all flags
    std::for_each(coordinates.begin(), coordinates.end(),
        [](auto& coord) {
            coord.isTETopEdge = false;
            coord.isTEBottomEdge = false;
        });

    auto [teteIt, tebeIt] = findTrailingEdgePoints(coordinates);

    if (teteIt != coordinates.end()) teteIt->isTETopEdge = true;
    if (tebeIt != coordinates.end()) tebeIt->isTEBottomEdge = true;
}

std::pair<std::vector<AirfoilCoordinate>, std::vector<AirfoilCoordinate>> AirfoilGeometryData::separateTopBottom(const std::vector<AirfoilCoordinate>& nodes) {
    std::vector<AirfoilCoordinate> top, bottom;

    for (const auto& node : nodes) {
        if (node.isTopSurface) {
            if (node.x == 1 && node.isTETopEdge == false) {
                continue;
            }
            else {
                top.push_back(node);
            }
            
        }
        else {
            if (node.x == 1 && node.isTEBottomEdge == false) {
                continue;
            }
            else {
                bottom.push_back(node);
            }
        }
    }

    // Sort upper surface: trailing edge to leading edge (x decreasing)
    std::sort(top.begin(), top.end(),
        [](const auto& a, const auto& b) {
            return a.x > b.x;  // Descending x
        });

    // Sort lower surface: leading edge to trailing edge (x increasing)
    std::sort(bottom.begin(), bottom.end(),
        [](const auto& a, const auto& b) {
            return a.x < b.x;  // Ascending x
        });

    // Check for first and last points 
    // for top -> first (1, 0) and last (0,0)
    // for bottom -> first (0,0) and last (1,0)
    // if there are these points missing append , insert them
    AirfoilCoordinate nodeTopFirst = AirfoilCoordinate(0, 1, 0, 0, true, true, true, false);
    AirfoilCoordinate nodeTopLast = AirfoilCoordinate(0, 0, 0, 0, true, false, false, false);
    AirfoilCoordinate nodeBottomFirst = AirfoilCoordinate(0, 0, 0, 0, false, false, false, false);
    AirfoilCoordinate nodeBottomLast = AirfoilCoordinate(0, 0, 0, 0, false, true, false, true);

    // check top
    if (top.front().x != 1.0) {
        top.insert(top.begin(), nodeTopFirst);
    }
    if (top.back().x != 0.0) {
        top.push_back(nodeTopLast);
    }
    // check bottom
    if (bottom.front().x != 0.0) {
        bottom.insert(bottom.begin(), nodeBottomFirst);
    }
    if (bottom.back().x != 1.0) {
        bottom.push_back(nodeBottomLast);
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
        xSet.insert(node.x);
    }

    // Convert to sorted vector
    std::vector<double> xCoords(xSet.begin(), xSet.end());
    return xCoords;
}

// Helper function to interpolate a single surface
std::vector<AirfoilCoordinate> AirfoilGeometryData::interpolateSurface(
    const std::vector<AirfoilCoordinate>& surface1, //thick
    const std::vector<AirfoilCoordinate>& surface2, //thin
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
    //for (double x : x1) {
        // Interpolate y-values at this x-coordinate
        double y1_interp = MathUtility::linearInterpolation(x,x1, y1);
        double y2_interp = MathUtility::linearInterpolation(x,x2, y2);

        // Linear interpolation between the two surfaces
        double y_final = (1.0 - percent) * y1_interp + percent * y2_interp;

        // Create node (adjust x order for top surface)
        //double x_final = isTopSurface ? xCoords[xCoords.size() - 1 - (indexCounter % xCoords.size())] : x;
        double x_final = x;
        result.emplace_back(indexCounter++, x_final, y_final, 0,isTopSurface, false);
    }
    if (isTopSurface) {
        std::reverse(result.begin(), result.end());
    }

    return result;
}

// Helper function to combine top and bottom surfaces
std::vector<AirfoilCoordinate> AirfoilGeometryData::combineTopBottomSurfaces( std::vector<AirfoilCoordinate>& top,
     std::vector<AirfoilCoordinate>& bottom) {
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


