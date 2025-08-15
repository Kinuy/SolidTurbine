#include "AirfoilPerformanceData.h"


void AirfoilPerformanceData::setName(const std::string& refNum) { name = refNum; }

void AirfoilPerformanceData::setXa(double xa) { xa = xa; }

void AirfoilPerformanceData::setRelativeThickness(double thickness) { thickness = thickness; }

void AirfoilPerformanceData::setReynoldsNumber(double reynolds) { reynolds = reynolds; }

void AirfoilPerformanceData::setDepang(double depang) { depang = depang; }

void AirfoilPerformanceData::setNAlpha(int nAlpha) { nAlpha = nAlpha; }

void AirfoilPerformanceData::setNVals(int nVals) { nVals = nVals; }

void AirfoilPerformanceData::addHeader(const std::string& header) {
    headers.push_back(header);
}

void AirfoilPerformanceData::addPerformancePoint(double alpha, double cl, double cd, double cm) {
    performancePoints.emplace_back(alpha, cl, cd, cm);
}

const std::string& AirfoilPerformanceData::getName() const { return name; }

double AirfoilPerformanceData::getXa() const { return xa; }

double AirfoilPerformanceData::getRelativeThickness() const { return relativeThickness; }

double AirfoilPerformanceData::getReynoldsNumber() const { return reynoldsNumber; }

double AirfoilPerformanceData::getDepang() const { return depang; }

int AirfoilPerformanceData::getNAlpha() const { return nAlpha; }

int AirfoilPerformanceData::getNVals() const { return nVals; }

const std::vector<AirfoilPerformancePoint>& AirfoilPerformanceData::getPerformanceData() const { return performancePoints; }

const std::vector<std::string>& AirfoilPerformanceData::getHeaders() const { return headers; }

std::string AirfoilPerformanceData::getTypeName() const { return "AirfoilPerformance"; }

size_t AirfoilPerformanceData::getRowCount() const { return performancePoints.size(); }

AirfoilPerformancePoint AirfoilPerformanceData::getPerformanceAtAlpha(double targetAlpha, double tolerance) const {
    // Find exact match first
    auto it = std::find_if(performancePoints.begin(), performancePoints.end(),
        [targetAlpha, tolerance](const AirfoilPerformancePoint& row) {
            return std::abs(row.alpha - targetAlpha) <= tolerance;
        });

    if (it != performancePoints.end()) {
        return *it;
    }

    throw std::runtime_error("No performance data found for alpha: " + std::to_string(targetAlpha));
}

AirfoilPerformancePoint AirfoilPerformanceData::interpolatePerformance(double targetAlpha) const {
    if (performancePoints.empty()) {
        throw std::runtime_error("No performance data available for interpolation");
    }

    // Find surrounding points
    auto lower = std::lower_bound(performancePoints.begin(), performancePoints.end(), targetAlpha,
        [](const AirfoilPerformancePoint& row, double alpha) {
            return row.alpha < alpha;
        });

    if (lower == performancePoints.begin()) {
        return performancePoints.front(); // Below range, return first point
    }
    if (lower == performancePoints.end()) {
        return performancePoints.back(); // Above range, return last point
    }

    auto upper = lower;
    --lower;

    // Linear interpolation
    double fraction = (targetAlpha - lower->alpha) / (upper->alpha - lower->alpha);
    double cl = lower->cl + fraction * (upper->cl - lower->cl);
    double cd = lower->cd + fraction * (upper->cd - lower->cd);
    double cm = lower->cm + fraction * (upper->cm - lower->cm);

    return AirfoilPerformancePoint(targetAlpha, cl, cd, cm);
}

double AirfoilPerformanceData::getMaxClAlpha() const {
    auto maxIt = std::max_element(performancePoints.begin(), performancePoints.end(),
        [](const AirfoilPerformancePoint& a, const AirfoilPerformancePoint& b) {
            return a.cl < b.cl;
        });
    return maxIt != performancePoints.end() ? maxIt->alpha : 0.0;
}

double AirfoilPerformanceData::getMaxCl() const {
    auto maxIt = std::max_element(performancePoints.begin(), performancePoints.end(),
        [](const AirfoilPerformancePoint& a, const AirfoilPerformancePoint& b) {
            return a.cl < b.cl;
        });
    return maxIt != performancePoints.end() ? maxIt->cl : 0.0;
}

double AirfoilPerformanceData::getMinCd() const {
    auto minIt = std::min_element(performancePoints.begin(), performancePoints.end(),
        [](const AirfoilPerformancePoint& a, const AirfoilPerformancePoint& b) {
            return a.cd < b.cd;
        });
    return minIt != performancePoints.end() ? minIt->cd : 0.0;
}

double AirfoilPerformanceData::getStallAlpha() const {
    // Simplified stall detection - maximum Cl point
    return getMaxClAlpha();
}

std::vector<double> AirfoilPerformanceData::getAlphaRange() const {
    std::vector<double> alphas;
    for (const auto& row : performancePoints) {
        alphas.push_back(row.alpha);
    }
    return alphas;
}