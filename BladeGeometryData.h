#pragma once

#include<vector>
#include<stdexcept>

#include "BladeGeometryRow.h"
#include "IStructuredData.h"

class BladeGeometryData : public IStructuredData {
private:
    std::vector<BladeGeometryRow> rows;
    std::vector<std::string> headers;

public:
    void addHeader(const std::string& header);

    void addRow(const BladeGeometryRow& row);

    const std::vector<BladeGeometryRow>& getRows() const;
    
    const std::vector<std::string>& getHeaders() const;

    std::string getTypeName() const;
    
    size_t getRowCount() const;

    BladeGeometryRow getRowByRadius(double radius, double tolerance) const;

    std::vector<double> getRadiusValues() const;
};
