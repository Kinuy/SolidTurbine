#pragma once

#define _USE_MATH_DEFINES

#include <vector>
#include <memory>
#include <math.h>
#include <stdexcept>
#include <iostream>
#include <fstream>

#include "BladeGeometryData.h"
#include "AirfoilGeometryData.h"
#include "AirfoilCoordinate.h"
#include "BladeGeometrySection.h"
#include "IInterpolationStrategy.h"
#include "InterpolationMethod.h"
#include "InterpolatedBladeSection.h"
#include "LinearInterpolationStrategy.h"
#include "CubicSplineInterpolationStrategy.h"
#include "AkimaSplineInterpolationStrategy.h"
#include "MonotonicCubicSplineInterpolationStrategy.h"


/**
 * @brief Blade geometry interpolation engine with configurable strategies
 *
 * BladeGeometryInterpolator provides comprehensive blade geometry interpolation
 * capabilities, combining blade section data with airfoil geometries to generate
 * smooth blade surfaces. Supports multiple interpolation methods through strategy
 * pattern and offers various output formats for CAD/CFD applications.
 *
 * ## Key Features
 * - **Strategy Pattern**: Pluggable interpolation algorithms (linear, spline, etc.)
 * - **3D Transformation**: Scales, rotates, and positions airfoil coordinates
 * - **Airfoil Matching**: Automatic selection based on thickness criteria
 * - **Dense Generation**: Creates high-resolution blade sections
 * - **Volume Calculation**: Approximates blade volume using integration
 * - **Export Capabilities**: Output for external tools and visualization
 *
 * ## Typical Workflow
 * 1. Initialize with blade geometry data and airfoil library
 * 2. Select interpolation method (can be changed dynamically)
 * 3. Generate interpolated sections or query specific radii
 * 4. Export results for downstream applications
 *
 * @see IInterpolationStrategy for available interpolation methods
 * @see BladeGeometryData for input blade section data
 * @see AirfoilGeometryData for airfoil coordinate data
 *
 * @example
 * ```cpp
 * BladeGeometryInterpolator interpolator(bladeData, airfoils,
 *                                       InterpolationMethod::CUBIC_SPLINE);
 * auto section = interpolator.interpolateAtRadius(0.75);
 * auto surface = interpolator.generateBladeSurface();
 * interpolator.exportBladeGeometry("blade.dat");
 * ```
 */
class BladeGeometryInterpolator {

private:

    /**
     * @brief Reference to blade geometry data containing radial sections
     *
     * Provides the base geometric data (radius, chord, twist, thickness)
     * for all radial stations along the blade span.
     */
    const BladeGeometryData& bladeGeometry;

    /**
     * @brief Collection of available airfoil geometries for section matching
     *
     * Vector of pointers to airfoil coordinate data used for selecting
     * appropriate airfoil shapes based on blade section requirements.
     */
    const std::vector<const AirfoilGeometryData*> airfoilGeometries;

    /**
     * @brief Current interpolation strategy for parameter interpolation
     *
     * Strategy pattern implementation allowing dynamic switching between
     * different interpolation algorithms (linear, cubic spline, etc.).
     */
    std::unique_ptr<IInterpolationStrategy> interpolationStrategy;

    /**
     * @brief Transforms airfoil coordinates to 3D blade position with scaling, rotation, and translation
     * @param coord Original airfoil coordinate
     * @param chord Chord length for scaling
     * @param twist Twist angle in degrees for rotation
     * @param radius Radial position for translation
     * @return Transformed 3D coordinate
     */
    AirfoilCoordinate transformCoordinate(const AirfoilCoordinate& coord,
        double chord, double twist, double radius) const;

    /**
     * @brief Selects the best matching airfoil for a blade section based on thickness
     * @param section Blade geometry section requiring airfoil selection
     * @return Pointer to best matching airfoil geometry
     * @throws std::runtime_error if no airfoil geometries available
     */
    const AirfoilGeometryData* selectAirfoilForSection(const BladeGeometrySection& section) const;

    /**
     * @brief Factory method creating interpolation strategy based on method type
     * @param method Interpolation method enum value
     * @return Unique pointer to interpolation strategy implementation
     */
    std::unique_ptr<IInterpolationStrategy> createInterpolationStrategy(InterpolationMethod method) const;

public:

    /**
     * @brief Constructor initializing interpolator with blade geometry, airfoils, and method
     * @param bladeGeom Blade geometry data
     * @param airfoilGeoms Vector of airfoil geometry pointers
     * @param method Interpolation method to use
     */
    BladeGeometryInterpolator(const BladeGeometryData& bladeGeom,
        const std::vector<const AirfoilGeometryData*>& airfoilGeoms,
        InterpolationMethod method = InterpolationMethod::LINEAR);

    /**
     * @brief Changes the interpolation method dynamically
     * @param method New interpolation method to use
     */
    void setInterpolationMethod(InterpolationMethod method);

    /**
     * @brief Gets the name of the currently active interpolation method
     * @return String name of current interpolation strategy
     */
    std::string getCurrentInterpolationMethod() const;

    /**
     * @brief Interpolates all blade sections from geometry data
     * @return Vector of interpolated blade sections
     * @note Continues processing even if individual sections fail
     */
    std::vector<InterpolatedBladeSection> interpolateAllSections() const;

    /**
     * @brief Interpolates a single blade section with airfoil coordinate transformation
     * @param section Blade geometry section to interpolate
     * @return Interpolated section with scaled and transformed coordinates
     */
    InterpolatedBladeSection interpolateSection(const BladeGeometrySection& section) const;

    /**
     * @brief Generates 3D blade surface points for visualization or meshing
     * @return 2D vector of coordinates representing blade surface
     */
    std::vector<std::vector<AirfoilCoordinate>> generateBladeSurface() const;

    /**
     * @brief Calculates approximate blade volume using trapezoidal integration
     * @return Blade volume in cubic units
     * @note Uses simplified cross-sectional area calculation
     */
    double calculateBladeVolume() const;

    /**
     * @brief Interpolates blade parameters at a specific radius using selected method
     * @param targetRadius Radius at which to interpolate blade properties
     * @return Interpolated blade section at target radius
     * @note Falls back to nearest section if interpolation fails
     */
    InterpolatedBladeSection interpolateAtRadius(double targetRadius) const;

    /**
     * @brief Generates densely spaced blade sections using interpolation
     * @param numSections Number of sections to generate between min/max radius
     * @return Vector of interpolated sections with uniform radial spacing
     * @throws std::runtime_error if no blade geometry data available
     */
    std::vector<InterpolatedBladeSection> generateDenseBladeSections(size_t numSections) const;

    /**
     * @brief Compares interpolation results from different methods at target radius
     * @param targetRadius Radius for comparison evaluation
     * @note Outputs comparison results to console
     */
    void compareInterpolationMethods(double targetRadius) const;

    /**
     * @brief Exports interpolated blade geometry to text file for CAD/CFD tools
     * @param filename Output file path
     * @throws std::runtime_error if file cannot be created
     */
    void exportBladeGeometry(const std::string& filename) const;

};

