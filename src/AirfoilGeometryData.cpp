#include "AirfoilGeometryData.h"
#include <set>
#include "MathUtility.h"
#include "AirfoilPolarData.h"
#include <algorithm>
#include <iostream>
#include <numbers>

void AirfoilGeometryData::setName(const std::string& n) { name = n; }



void AirfoilGeometryData::setRelativeThickness(double thickness) { relativeThickness = thickness; }

void AirfoilGeometryData::setZCoordinates(double z)
{
    for (auto& coord : coordinates) {
        coord.z = z;
    }
}

void AirfoilGeometryData::setMarkerIndex(std::string type, int idx)
{
    for (auto& marker : markers) {
        if (marker.type == type) {
            marker.index = idx;
        }
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

void AirfoilGeometryData::addAirfoilNosePoint(const bool nosePointExists)
{
    if(!nosePointExists){
        std::cout << "Warning: Airfoil nose point (0,0) not found in geometry data. Adding it automatically." << std::endl;

        if (coordinates.empty()) return;

        // Find minimum x point
        auto minXIt = std::min_element(coordinates.begin(), coordinates.end(),
            [](const AirfoilCoordinate& a, const AirfoilCoordinate& b) {
                return a.x < b.x;
            });

        int insertPos = std::distance(coordinates.begin(), minXIt) + 1;

        // Create origin point and copy
        AirfoilCoordinate originPoint(0, 0.0, 0.0, 0.0, true, false, false,false);
        AirfoilCoordinate minXCopy = *minXIt;

        // Insert both points at once
        coordinates.insert(coordinates.begin() + insertPos, { originPoint, minXCopy });

        // Update topBottom falg of copied point
	    coordinates[insertPos + 1].isTopSurface = false;

        // Update all indices after insertion
        for (size_t i = 0; i < coordinates.size(); ++i)
        {
            coordinates[i].index = i;
        }
    }
}

void AirfoilGeometryData::insertNosePointAtOrigin(const bool nosePointExists)
{
    if (!nosePointExists) {
        if (coordinates.empty()) return;

        // Find point with minimum x value (nose point)
        auto minXIt = std::min_element(coordinates.begin(), coordinates.end(),
            [](const AirfoilCoordinate& a, const AirfoilCoordinate& b) {
                return a.x < b.x;
            });

        // Create new point at x=0 with same y as nose point
        AirfoilCoordinate nosePoint(
            0,                  // index (will be updated later)
            0.0,               // x = 0
            minXIt->y,         // same y as minimum x point
            minXIt->z,         // same z
            minXIt->isTopSurface,
            false,             // not trailing edge
            false,
            false
        );

        // Insert at the position of minimum x point
        size_t insertPos = std::distance(coordinates.begin(), minXIt);
        coordinates.insert(coordinates.begin() + insertPos, nosePoint);

        // Update indices
        for (size_t i = 0; i < coordinates.size(); ++i) {
            coordinates[i].index = i;
        }
    }
}

void AirfoilGeometryData::insertNosePointWithInterpolation(const bool nosePointExists)
{
    if (!nosePointExists) {
        if (coordinates.size() < 2) return;

        // Find the point with minimum x
        auto minXIt = std::min_element(coordinates.begin(), coordinates.end(),
            [](const AirfoilCoordinate& a, const AirfoilCoordinate& b) {
                return a.x < b.x;
            });

        // Find a nearby point to interpolate with
        double interpolatedY = minXIt->y; // default to same Y

        // Try to find another point close to the minimum x for better interpolation
        if (coordinates.size() > 1) {
            auto secondClosest = coordinates.begin();
            for (auto it = coordinates.begin(); it != coordinates.end(); ++it) {
                if (it != minXIt &&
                    std::abs(it->x) < std::abs(secondClosest->x) &&
                    secondClosest == minXIt) {
                    secondClosest = it;
                }
                else if (it != minXIt &&
                    std::abs(it->x) < std::abs(secondClosest->x)) {
                    secondClosest = it;
                }
            }

            if (secondClosest != minXIt) {
                // Linear interpolation to x=0
                double x1 = minXIt->x, y1 = minXIt->y;
                double x2 = secondClosest->x, y2 = secondClosest->y;
                if (x1 != x2) {
                    interpolatedY = y1 + (0.0 - x1) * (y2 - y1) / (x2 - x1);
                }
            }
        }

        AirfoilCoordinate nosePoint(0, 0.0, interpolatedY, minXIt->z, true, false, false, false);

        size_t insertPos = std::distance(coordinates.begin(), minXIt);
        coordinates.insert(coordinates.begin() + insertPos, nosePoint);

        // Update indices
        for (size_t i = 0; i < coordinates.size(); ++i) {
            coordinates[i].index = i;
        }
    }
}

void AirfoilGeometryData::moveAllCoordinatesByNoseXY(const bool centeredNosePointExists)
{
    if (!centeredNosePointExists) {
        // Find the point with minimum x
        auto minXIt = std::min_element(coordinates.begin(), coordinates.end(),
            [](const AirfoilCoordinate& a, const AirfoilCoordinate& b) {
                return a.x < b.x;
            });
        // Move all points by x,y of nose point except TE points (1,y) -> they are only moved by y
        for (auto& coord : coordinates) {
            // If coordinate is at TE move just y vals
            if (coord.isTrailingEdge) {
                    coord.y -= std::abs(minXIt->y);
            }
            else {
                coord.x -= std::abs(minXIt->x);
                coord.y -= std::abs(minXIt->y);
            }
        }
    }
}

void AirfoilGeometryData::orientationToDefaultCounterClockwiseOrientation()
{
    // Is orentation counterclockwise?
    if (coordinates.at(0).y > coordinates.at(1).y) {
        std::reverse(coordinates.begin(), coordinates.end());
    }
    coordinateOrientation = true;
}

void AirfoilGeometryData::findAndAssignLE()
{
    int idx = 0;
    for (auto& coord : coordinates) {  
        if (coord.x == 0) {
            std::for_each(markers.begin(),
                markers.end(),
                [idx](AirfoilMarker& m) {
                    if (m.type == "LE") m.index = idx;
                });
            return;
        }
        idx++;
    }
}

void AirfoilGeometryData::findAndAssignTopBottom()
{
    if (coordinateOrientation) {
        int idx = 0;
        for (auto& coord : coordinates) {
            int indxLE = getMarkerByType("LE").index;
            if (idx <= indxLE) {
                coord.isTopSurface = true;
            }
            idx++;
        }
    }
}

const std::string& AirfoilGeometryData::getName() const { return name; }

double AirfoilGeometryData::getRelativeThickness() const { return relativeThickness; }

const std::vector<AirfoilMarker>& AirfoilGeometryData::getMarkers() const { return markers; }

const std::vector<AirfoilCoordinate>& AirfoilGeometryData::getCoordinates() const { return coordinates; }

const std::vector<AirfoilCoordinate>& AirfoilGeometryData::getScaledAndRotatedCoordinates() const { return scaledCoordinates; }

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

bool AirfoilGeometryData::coordinateIsTop(const int idx, const int topSide) const
{
    // TODO: Perhabs here is a more sophisticated approach needed than hardcoded index boundary....
    // Sometimes top trailing edge points have negative y values so this case has to be handled
    if (idx <= topSide) {
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
        [[maybe_unused]] double maxXUpper = std::numeric_limits<double>::lowest();
        [[maybe_unused]] double maxXLower = std::numeric_limits<double>::lowest();

        for (auto it = coordinates.begin(); it != coordinates.end(); ++it) {
            auto index = std::distance(coordinates.begin(), it);
            if (index < 50 && (it->y >= 0.0 || it->y < 0.0) && it->x >= 1.0) {
                maxXUpper = it->x;
                teteIt = it;
            }
            else if (index > 50 && (it->y >= 0.0 || it->y < 0.0) && it->x >= 1.0) {
                maxXLower = it->x;
                tebeIt = it;
                break;
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
    teteIt->isTETopEdge = true;
    tebeIt->isTEBottomEdge = true;
    //if (teteIt != coordinates.end()) teteIt->isTETopEdge = true;
    //if (tebeIt != coordinates.end()) tebeIt->isTEBottomEdge = true;
}

void AirfoilGeometryData::applyTwistAroundQuarterChord(double twistAngleDegrees, double pitchAxis)
{
    if (scaledCoordinates.empty()) return;

    auto [minIt, maxIt] = std::minmax_element(coordinates.begin(), coordinates.end(),
        [](const AirfoilCoordinate& a, const AirfoilCoordinate& b) {
            return a.x < b.x;
        });

    double quarterChordX = minIt->x + (0.25 + pitchAxis / 100.0) * (maxIt->x - minIt->x);
    double quarterChordY = 0.0;

    applyTwistAngleAroundPivotPoint(twistAngleDegrees, quarterChordX, quarterChordY);
}

void AirfoilGeometryData::applyTwistAngleAroundPivotPoint(double twistAngleDegrees, double pivotX, double pivotY)
{
    if (scaledCoordinates.empty()) return;

    // Convert degrees to radians
    double angleRad = twistAngleDegrees * std::numbers::pi / 180.0;

    // 2D rotation matrix
    double rotMatrix[2][2] = {
        {std::cos(angleRad), -std::sin(angleRad)},
        {std::sin(angleRad),  std::cos(angleRad)}
    };

    for (auto& coord : coordinates) {
        // Translate to pivot point
        double xTemp = coord.x - pivotX;
        double yTemp = coord.y - pivotY;

        // Apply rotation matrix
        double xNew = rotMatrix[0][0] * xTemp + rotMatrix[0][1] * yTemp;
        double yNew = rotMatrix[1][0] * xTemp + rotMatrix[1][1] * yTemp;

        // Translate back
        coord.x = xNew + pivotX;
        coord.y = yNew + pivotY;
    }
}

void AirfoilGeometryData::applyScalingWithChordAndMaxThickness(double chordLength, double maxThickness, double targetRadius)
{
    // Copy original coordinates
	scaledCoordinates = coordinates;
	
    // Scale all coordinates
    for (auto& coord : scaledCoordinates) {
        coord.x *= chordLength;
        coord.y *= maxThickness;
		coord.z *= targetRadius;
	}
}

void AirfoilGeometryData::applyTranslationXY(double pcbaX, double pcbaY)
{
    // Translate all coordinates
    for (auto& coord : scaledCoordinates) {
        coord.x += pcbaY;
        coord.y += pcbaX;
    }
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
    AirfoilCoordinate nodeBottomLast = AirfoilCoordinate(0, 1, 0, 0, false, true, false, true);

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

std::vector<double> AirfoilGeometryData::getUniqueXCoordinates(const std::vector<AirfoilCoordinate>& surfaceThick,
    const std::vector<AirfoilCoordinate>& surfaceThin) {

    std::set<double> xSet;

    // Collect x-coordinates from both surfaces
    for (const auto& node : surfaceThick) {
        xSet.insert(node.x);
    }
    for (const auto& node : surfaceThin) {
        xSet.insert(node.x);
    }

    // Convert to sorted vector
    std::vector<double> xCoords(xSet.begin(), xSet.end());
    return xCoords;
}

// Helper function to interpolate a single surface
std::vector<AirfoilCoordinate> AirfoilGeometryData::interpolateSurface(
    const std::vector<AirfoilCoordinate>& surfaceThick,
    const std::vector<AirfoilCoordinate>& surfaceThin,
    const std::vector<double>& xCoords,
    double percent,
    bool isTopSurface) {

    // Extract x,y coordinates
    std::vector<double> xThick, yThick, xThin, yThin;
    for (const auto& node : surfaceThick) {
        xThick.push_back(node.x);
        yThick.push_back(node.y);
    }
    for (const auto& node : surfaceThin) {
        xThin.push_back(node.x);
        yThin.push_back(node.y);
    }

    // For top surface, reverse for proper interpolation direction
    if (isTopSurface) {
        std::reverse(xThick.begin(), xThick.end());
        std::reverse(yThick.begin(), yThick.end());
        std::reverse(xThin.begin(), xThin.end());
        std::reverse(yThin.begin(), yThin.end());
    }

    std::vector<AirfoilCoordinate> result;
    int indexCounter = 0;

    for (double x_final : xCoords) {
        // Interpolate y-values at this x-coordinate
        double yThick_interp = MathUtility::linearInterpolation(x_final, xThick, yThick);
        double yThin_interp = MathUtility::linearInterpolation(x_final, xThin, yThin);

        // Linear interpolation between the two surfaces
        double y_final = (1.0 - percent) * yThick_interp + percent * yThin_interp;

        // Create node (adjust x order for top surface)
        result.emplace_back(indexCounter++, x_final, y_final, 0, isTopSurface, false, false, false);
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
    //combined.emplace_back(indexCounter++, 1.0, 0.0, true, false);

    // Add top surface (excluding endpoints to avoid duplicates)
    for (size_t i = 0; i < top.size(); ++i) {
        //if (top[i].x != 1.0 || top[i].y != 0.0) { // Skip if it's the endpoint
            combined.emplace_back(indexCounter++, top[i].x, top[i].y, 0, true, false);
        //}
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

    // Calculate interpolation factor (0 = thick airfoil, 1 = thin airfoil)
    double percent = std::abs((targetThickness - thickThickness) / (thinThickness - thickThickness));

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


