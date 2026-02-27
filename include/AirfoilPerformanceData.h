#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "IStructuredData.h"
#include "AirfoilPerformancePoint.h"

/**
 * @brief Structured data container for airfoil aerodynamic performance datasets
 *
 * AirfoilPerformanceData implements IStructuredData to store and manage complete
 * airfoil performance datasets including aerodynamic coefficients across angle
 * of attack ranges. Provides both metadata access and performance data retrieval
 * with interpolation capabilities for aerodynamic analysis.
 *
 * ## Key Features
 * - **Metadata Storage**: Airfoil parameters (thickness, Reynolds number, etc.)
 * - **Performance Data**: Lift, drag, and moment coefficients vs angle of attack
 * - **Interpolation**: Linear interpolation for intermediate angle values
 * - **Analysis Methods**: Maximum/minimum coefficient extraction and stall detection
 *
 * ## Typical Data Structure
 * Contains coefficient data (Cl, Cd, Cm) across angle of attack range for
 * specific Reynolds number and airfoil configuration, commonly used in
 * blade element momentum (BEM) analysis and CFD validation.
 *
 * ## Use Cases
 * - Wind turbine blade aerodynamic analysis
 * - Airfoil performance comparison and selection
 * - BEM theory calculations and lookup tables
 * - CFD validation and verification studies
 *
 * @see IStructuredData for the base interface
 * @see AirfoilPerformancePoint for individual data point structure
 * @see AirfoilPerformanceParser for file parsing
 * 
 * @deprecated Use AirfoilPolarData instead for new implementations.
 *
 * @example
 * ```cpp
 * AirfoilPerformanceData airfoil;
 * airfoil.setName("NACA0012");
 * airfoil.addPerformancePoint(5.0, 0.8, 0.02, -0.1);
 * auto interpolated = airfoil.interpolatePerformance(7.5);
 * ```
 */
 // TODO: Remove calss in future versions, use AirfoilPolarData instead
class AirfoilPerformanceData : public IStructuredData {

private:

    /**
     * @brief Airfoil reference name or identifier
     */
    std::string name;

    /**
     * @brief Xa parameter (airfoil-specific parameter)
     */
    double xa;

    /**
     * @brief Relative thickness as percentage of chord
     */
    double relativeThickness;

    /**
     * @brief Reynolds number for this performance dataset
     */
    double reynoldsNumber;

    /**
     * @brief Depang parameter (angle-related parameter)
     */
    double depang;

    /**
     * @brief Number of angle of attack data points
     */
    int nAlpha;

    /**
     * @brief Number of values per data point
     */
    int nVals;

    /**
     * @brief Collection of aerodynamic performance data points
     */
    std::vector<AirfoilPerformancePoint> performancePoints;

    /**
     * @brief Header lines from source file (comments and metadata)
     */
    std::vector<std::string> headers;

public:

    /**
     * @brief Sets the airfoil reference name/identifier
     * @param refNum Reference number or name string
     */
    void setName(const std::string& refNum);

    /**
     * @brief Sets the xa parameter value
     * @param xa Xa parameter value
     */
    void setXa(double xa);

    /**
     * @brief Sets the relative thickness percentage
     * @param thickness Relative thickness value
     */
    void setRelativeThickness(double thickness);

    /**
     * @brief Sets the Reynolds number for this dataset
     * @param reynolds Reynolds number value
     */
    void setReynoldsNumber(double reynolds);

    /**
     * @brief Sets the depang parameter value
     * @param depang Depang parameter value
     */
    void setDepang(double depang);

    /**
     * @brief Sets the number of alpha points
     * @param nAlpha Number of angle of attack data points
     */
    void setNAlpha(int nAlpha);

    /**
     * @brief Sets the number of values per data point
     * @param nVals Number of values per point
     */
    void setNVals(int nVals);

    /**
     * @brief Adds a header line to the dataset
     * @param header Header string to add
     */
    void addHeader(const std::string& header);

    /**
     * @brief Adds a performance data point
     * @param alpha Angle of attack [degrees]
     * @param cl Lift coefficient
     * @param cd Drag coefficient
     * @param cm Moment coefficient
     */
    void addPerformancePoint(double alpha, double cl, double cd, double cm);

    /**
     * @brief Gets the airfoil name/reference
     * @return Airfoil reference string
     */
    const std::string& getName() const;

    /**
     * @brief Gets the xa parameter value
     * @return Xa parameter value
     */
    double getXa() const;

    /**
     * @brief Gets the relative thickness percentage
     * @return Relative thickness value
     */
    double getRelativeThickness() const;

    /**
     * @brief Gets the Reynolds number
     * @return Reynolds number value
     */
    double getReynoldsNumber() const;

    /**
     * @brief Gets the depang parameter value
     * @return Depang parameter value
     */
    double getDepang() const;

    /**
     * @brief Gets the number of alpha points
     * @return Number of angle of attack data points
     */
    int getNAlpha() const;

    /**
     * @brief Gets the number of values per data point
     * @return Number of values per point
     */
    int getNVals() const;

    /**
     * @brief Gets all performance data points
     * @return Const reference to vector of AirfoilPerformancePoint objects
     */
    const std::vector<AirfoilPerformancePoint>& getPerformanceData() const;

    /**
     * @brief Gets all header lines
     * @return Const reference to vector of header strings
     */
    const std::vector<std::string>& getHeaders() const;

    /**
     * @brief Gets the data type identifier
     * @return Always returns "AirfoilPerformance"
     */
    std::string getTypeName() const override;

    /**
     * @brief Gets the number of performance data points
     * @return Number of performance points in dataset
     */
    size_t getRowCount() const override;


    /**
     * @brief Finds performance data at specific angle of attack with tolerance
     * @param targetAlpha Target angle of attack [degrees]
     * @param tolerance Acceptable deviation from target [degrees]
     * @return AirfoilPerformancePoint at specified angle
     * @throws std::runtime_error if no matching data found within tolerance
     */
    AirfoilPerformancePoint getPerformanceAtAlpha(double targetAlpha, double tolerance = 0.1) const;

    /**
     * @brief Interpolates performance data at target angle of attack
     * @param targetAlpha Target angle of attack [degrees]
     * @return Interpolated AirfoilPerformancePoint
     * @throws std::runtime_error if no data available for interpolation
     * @note Returns boundary values for targets outside data range
     */
    AirfoilPerformancePoint interpolatePerformance(double targetAlpha) const;

    /**
     * @brief Gets angle of attack at maximum lift coefficient
     * @return Alpha at maximum Cl [degrees]
     */
    double getMaxClAlpha() const;

    /**
     * @brief Gets maximum lift coefficient value
     * @return Maximum Cl value in dataset
     */
    double getMaxCl() const;

    /**
     * @brief Gets minimum drag coefficient value
     * @return Minimum Cd value in dataset
     */
    double getMinCd() const;

    /**
     * @brief Gets estimated stall angle of attack
     * @return Stall angle [degrees] (currently returns angle at max Cl)
     * @note Simplified implementation using maximum Cl point
     */
    double getStallAlpha() const;

    /**
     * @brief Gets all angle of attack values from dataset
     * @return Vector of alpha values [degrees]
     */
    std::vector<double> getAlphaRange() const;


};


