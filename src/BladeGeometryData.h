#pragma once

#include<vector>
#include<stdexcept>

#include "BladeGeometrySection.h"
#include "IStructuredData.h"

class BladeGeometryData : public IStructuredData {
private:
    std::vector<BladeGeometrySection> rows;
    std::vector<std::string> headers;

public:
    void addHeader(const std::string& header);

    void addRow(const BladeGeometrySection& row);

    const std::vector<BladeGeometrySection>& getRows() const;
    
    const std::vector<std::string>& getHeaders() const;

    std::string getTypeName() const;
    
    size_t getRowCount() const;

    BladeGeometrySection getRowByRadius(double radius, double tolerance) const;

    BladeGeometrySection getRowByIndex(int index) const;

    std::vector<double> getRadiusValues() const;
};
