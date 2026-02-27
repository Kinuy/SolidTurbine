#pragma once

#include <string>
#include <vector>
#include <set>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include "MathUtility.h"

#include "AirfoilPolarPoint.h"
#include "IStructuredData.h"
#include "IInterpolationStrategy.h"
#include "LinearInterpolationStrategy.h"
#include "AirfoilAeroCoefficients.h"
#include "AirfoilOperationCondition.h"

/**
 * @brief Modern, simplified airfoil polar data class
 */
class AirfoilPolarData : public IStructuredData
{

private:
    /**
     * @brief Name of the airfoil
     *
     * Used for identification and reference in simulations.
     */
    std::string name;

    /**
     * @brief Relative thickness of the airfoil (as a percentage)
     *
     * Represents the maximum thickness divided by chord length, e.g., 0.12 for 12%.
     */
    double relativeThickness = 0.0;

    /**
     * @brief Collection of polar data points
     *
     * Stores the airfoil performance data points, each containing operation conditions
     * and corresponding aerodynamic coefficients.
     */
    std::vector<AirfoilPolarPoint> polarData;

    /**
     * @brief Interpolation strategy for coefficient estimation
     *
     * Uses the strategy pattern to allow different interpolation methods (e.g., linear, spline).
     * Default is linear interpolation for simplicity.
     *
     */
    std::unique_ptr<IInterpolationStrategy> interpolationStrategy;

    /**
     * @brief Header lines from source file (comments and metadata)
     */
    std::vector<std::string> headers;

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
     * @brief Xa parameter (airfoil-specific parameter)
     */
    double xa;

    /**
     * @brief Build interpolation grids for Reynolds, Mach, and angle of attack
     * @param reynolds Vector of Reynolds numbers
     * @param machs Vector of Mach numbers
     * @param alphas Vector of angles of attack
     * @param clGrid Output grid for lift coefficients
     * @param cdGrid Output grid for drag coefficients
     * @param cmGrid Output grid for moment coefficients
     */
    void buildInterpolationGrids(
        const std::vector<double> &reynolds,
        const std::vector<double> &machs,
        const std::vector<double> &alphas,
        std::vector<std::vector<std::vector<double>>> &clGrid,
        std::vector<std::vector<std::vector<double>>> &cdGrid,
        std::vector<std::vector<std::vector<double>>> &cmGrid) const;

    /**
     * @brief Build Mach and angle grids for interpolation
     *
     * Creates a 2D grid of Mach numbers and angles for each Reynolds number.
     * This simplifies the trilinear interpolation process.
     *
     * @param reynolds Vector of Reynolds numbers
     * @param machs Vector of Mach numbers
     * @return 2D vector of Mach numbers for each Reynolds number
     */
    std::vector<std::vector<double>> buildMachGrid(
        const std::vector<double> &reynolds,
        const std::vector<double> &machs) const;

    /**
     * @brief Build angle grid for interpolation
     *
     * Creates a 3D grid of angles of attack for each Reynolds/Mach combination.
     * This simplifies the trilinear interpolation process.
     *
     * @param reynolds Vector of Reynolds numbers
     * @param machs Vector of Mach numbers
     * @param alphas Vector of angles of attack
     * @return 3D vector of angles of attack for each Reynolds/Mach combination
     */
    std::vector<std::vector<std::vector<double>>> buildAlphaGrid(
        const std::vector<double> &reynolds,
        const std::vector<double> &machs,
        const std::vector<double> &alphas) const;

public:
    /**
     * @brief Find or interpolate coefficients for a given operation condition
     *
     * Searches for an exact match in polar data, or returns zero coefficients if not found.
     * Could be improved with nearest neighbor interpolation in the future.
     *
     * @param condition Airfoil operation condition to find coefficients for
     * @return Aerodynamic coefficients for the specified condition
     */
    AirfoilAeroCoefficients findOrInterpolateCoefficients(const AirfoilOperationCondition &condition) const;

    /**
     * @brief Default constructor
     */

    AirfoilPolarData();
    /**
     * @brief constructor
     * @param n Name or reference number for the airfoil polar data
     */
    AirfoilPolarData(std::string n);

    /**
     * @brief Simplified coefficient interpolation using modern approach
     * @param target Target operation condition for interpolation
     * @return Interpolated aerodynamic coefficients
     */
    AirfoilAeroCoefficients interpolateCoefficients(const AirfoilOperationCondition &target) const;

    /**
     * @brief Simplified trilinear interpolation
     * @param target Target operation condition for interpolation
     * @return Interpolated aerodynamic coefficients
     */
    AirfoilAeroCoefficients performTrilinearInterpolation(const AirfoilOperationCondition &target) const;

    /**
     * @brief Main simplified interpolation method
     * @param leftPolar Left polar data for interpolation
     * @param rightPolar Right polar data for interpolation
     * @param targetThickness Target relative thickness for the new polar
     * @return Interpolated polar data between two existing polars
     */
    static std::unique_ptr<AirfoilPolarData> interpolateBetweenPolars(
        const AirfoilPolarData &leftPolar,
        const AirfoilPolarData &rightPolar,
        double targetThickness);

    /**
     * @brief Create combined operating conditions from two polars
     * @return Vector of unique AirfoilOperationCondition objects
     */
    static std::vector<AirfoilOperationCondition> createCombinedConditions(
        const AirfoilPolarData &left,
        const AirfoilPolarData &right);

    /**
     * @brief Add new polar data point
     */
    void addPolarPoint(const AirfoilOperationCondition &condition, const AirfoilAeroCoefficients &coefficients);

    /**
     * @brief Sets the airfoil reference name/identifier
     * @param refNum Reference number or name string
     */
    void setName(const std::string &refNum);

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
    void addHeader(const std::string &header);

    /**
     * @brief Get the file path of the polar data
     * @return relative thickness of the airfoil
     */
    double getRelativeThickness() const;

    /**
     * @brief Get the stored polar data points
     * @return Vector of AirfoilPolarPoint objects containing operation conditions and coefficients
     */
    const std::vector<AirfoilPolarPoint> &getPolarData() const;

    /**
     * @brief Get all unique Reynolds numbers from the data
     * @return Vector of unique Reynolds numbers
     */
    std::vector<double> getReynoldsNumbers() const;

    /**
     * @brief Get all unique Mach numbers from the data
     * @return Vector of unique Mach numbers
     */
    std::vector<double> getMachNumbers() const;

    /**
     * @brief Get all unique angles of attack from the data
     * @return Vector of unique angles of attack
     */
    std::vector<double> getAnglesOfAttack() const;

    /**
     * @brief Gets the airfoil name/reference
     * @return Airfoil reference string
     */
    const std::string &getName() const;

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
     * @brief Gets the type name identifier for this data structure
     * @return String identifying the data type (e.g., "BladeGeometry", "AirfoilPerformance")
     */
    std::string getTypeName() const override;

    /**
     * @brief Gets the number of data rows/records in this structure
     * @return Number of rows/entries in the data collection
     */
    size_t getRowCount() const override;
};
