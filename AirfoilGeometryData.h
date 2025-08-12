#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "IStructuredData.h"
#include "AirfoilCoordinate.h"
#include "AirfoilMarker.h"
#include "AirfoilGeometryFileInfo.h"

// Responsible for managing Airfoil geometry data
class AirfoilGeometryData : public IStructuredData {

private:

    std::string name;

    double relativeThickness;

    std::vector<AirfoilMarker> markers;

    std::vector<AirfoilCoordinate> coordinates;

    std::vector<std::string> headers;

public:

    void setName(const std::string& name);
    void setRelativeThickness(double thickness);

    void addHeader(const std::string& header);

    void addMarker(const std::string& type, int index);

    void addCoordinate(double x, double y);

    const std::string& getName() const;

    double getRelativeThickness() const;

    const std::vector<AirfoilMarker>& getMarkers() const;

    const std::vector<AirfoilCoordinate>& getCoordinates() const;

    const std::vector<std::string>& getHeaders() const;

    std::string getTypeName() const override;

    size_t getRowCount() const override;

    AirfoilCoordinate getLeadingEdge() const;

    AirfoilCoordinate getTrailingEdge() const;

    std::vector<AirfoilCoordinate> getUpperSurface() const;

    std::vector<AirfoilCoordinate> getLowerSurface() const;

    double getChordLength() const;

    double getMaxThickness() const;

private:

    AirfoilMarker getMarkerByType(const std::string& type) const;

};

