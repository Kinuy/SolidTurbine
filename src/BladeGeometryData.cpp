#include "BladeGeometryData.h"

void BladeGeometryData::addHeader(const std::string& header) {
    headers.push_back(header);
}

void BladeGeometryData::addRow(BladeGeometrySection&& row) {
    rows.push_back(std::move(row));
}

const std::vector<BladeGeometrySection>& BladeGeometryData::getRows() const { return rows; }

const std::vector<std::string>& BladeGeometryData::getHeaders() const { return headers; }

std::string BladeGeometryData::getTypeName() const { return "BladeGeometry"; }

size_t BladeGeometryData::getRowCount() const { return rows.size(); }

//BladeGeometrySection& BladeGeometryData::getRowByRadius(double radius, double tolerance) const {
//    auto it = std::find_if(rows.begin(), rows.end(),
//        [radius, tolerance](const BladeGeometrySection& row) {
//            return std::abs(row.bladeRadius - radius) <= tolerance;
//        });
//
//    if (it == rows.end()) {
//        throw std::runtime_error("No blade geometry found for radius: " + std::to_string(radius));
//    }
//    return *it;
//}
//
//BladeGeometrySection& BladeGeometryData::getRowByRelativeThickness(double relativeThickness, double tolerance) const {
//    auto it = std::find_if(rows.begin(), rows.end(),
//        [relativeThickness, tolerance](const BladeGeometrySection& row) {
//            return std::abs(row.relativeThickness - relativeThickness) <= tolerance;
//        });
//
//    if (it == rows.end()) {
//        throw std::runtime_error("No blade geometry found for relativeThickness: " + std::to_string(relativeThickness));
//    }
//    return *it;
//}
//
//BladeGeometrySection& BladeGeometryData::getRowByIndex(int index) const{
//        return rows.at(index);
//}

std::vector<double> BladeGeometryData::getRadiusValues() const {
    std::vector<double> radii;
    for (const auto& row : rows) {
        radii.push_back(row.bladeRadius);
    }
    return radii;
}
