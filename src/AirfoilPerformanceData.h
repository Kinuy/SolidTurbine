#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "IStructuredData.h"
#include "AirfoilPerformancePoint.h"

// Responsible for managing airfoil performance data
class AirfoilPerformanceData : public IStructuredData {

private:

    std::string name;

    double xa;

    double relativeThickness;

    double reynoldsNumber;

    double depang;

    int nAlpha;

    int nVals;

    std::vector<AirfoilPerformancePoint> performancePoints;

    std::vector<std::string> headers;

public:

    void setName(const std::string& refNum);

    void setXa(double xa);

    void setRelativeThickness(double thickness);

    void setReynoldsNumber(double reynolds);

    void setDepang(double depang);

    void setNAlpha(int nAlpha);

    void setNVals(int nVals);

    void addHeader(const std::string& header);

    void addPerformancePoint(double alpha, double cl, double cd, double cm);

    const std::string& getName() const;

    double getXa() const;

    double getRelativeThickness() const;

    double getReynoldsNumber() const;

    double getDepang() const;

    int getNAlpha() const;

    int getNVals() const;

    const std::vector<AirfoilPerformancePoint>& getPerformanceData() const;

    const std::vector<std::string>& getHeaders() const;

    std::string getTypeName() const override;

    size_t getRowCount() const override;

    AirfoilPerformancePoint getPerformanceAtAlpha(double targetAlpha, double tolerance = 0.1) const;

    // Linear interpolation between two alpha values
    AirfoilPerformancePoint interpolatePerformance(double targetAlpha) const;

    double getMaxClAlpha() const;

    double getMaxCl() const;

    double getMinCd() const;

    double getStallAlpha() const;

    std::vector<double> getAlphaRange() const;

};


