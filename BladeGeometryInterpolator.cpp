#include "BladeGeometryInterpolator.h"

// Transform coordinates: scale by chord, rotate by twist, translate to radial position
AirfoilCoordinate BladeGeometryInterpolator::transformCoordinate(const AirfoilCoordinate& coord,
    double chord, double twist, double radius) const {
    // Scale by chord length
    double scaledX = coord.x * chord;
    double scaledY = coord.y * chord;

    // Convert twist to radians
    double twistRad = twist * M_PI / 180.0;

    // Rotate coordinates around chord line (assuming x is chordwise, y is thickness)
    double rotatedX = scaledX * cos(twistRad) - scaledY * sin(twistRad);
    double rotatedY = scaledX * sin(twistRad) + scaledY * cos(twistRad);

    // Translate to radial position (y becomes radial coordinate)
    return AirfoilCoordinate(rotatedX, rotatedY, radius); // x, y, z = r coordinates
}

// Find the most appropriate airfoil for a given blade section
const AirfoilGeometryData* BladeGeometryInterpolator::selectAirfoilForSection(const BladeGeometrySection& section) const {
    if (airfoilGeometries.empty()) {
        throw std::runtime_error("No airfoil geometries available for interpolation");
    }

    // Simple selection strategy: use thickness to match airfoil
    // In a real application, you'd have a more sophisticated mapping

    double targetThickness = section.relativeThickness;
    const AirfoilGeometryData* bestMatch = airfoilGeometries.at(0);
    double minThicknessDiff = std::abs(bestMatch->getRelativeThickness() - targetThickness);

    for (const auto* airfoil : airfoilGeometries) {
        double thicknessDiff = std::abs(airfoil->getRelativeThickness() - targetThickness);
        if (thicknessDiff < minThicknessDiff) {
            minThicknessDiff = thicknessDiff;
            bestMatch = airfoil;
        }
    }

    return bestMatch;
}

// Create interpolation strategy based on method
std::unique_ptr<IInterpolationStrategy> BladeGeometryInterpolator::createInterpolationStrategy(InterpolationMethod method) const {
    switch (method) {
    case InterpolationMethod::LINEAR:
        return std::make_unique<LinearInterpolationStrategy>();
    case InterpolationMethod::CUBIC_SPLINE:
        return std::make_unique<CubicSplineInterpolationStrategy>();
    case InterpolationMethod::AKIMA_SPLINE:
        return std::make_unique<AkimaSplineInterpolationStrategy>();
    case InterpolationMethod::MONOTONIC_CUBIC_SPLINE:
        return std::make_unique<MonotonicCubicInterpolationStrategy>();
    default:
        return std::make_unique<LinearInterpolationStrategy>();
    }
}

BladeGeometryInterpolator::BladeGeometryInterpolator(
    const BladeGeometryData& bladeGeom,
    std::vector<const AirfoilGeometryData*>& airfoilGeoms,
    InterpolationMethod method)
    : 
    bladeGeometry(bladeGeom), 
    airfoilGeometries(airfoilGeoms),
    interpolationStrategy(createInterpolationStrategy(method)) 
{
}

// Change interpolation method dynamically
void BladeGeometryInterpolator::setInterpolationMethod(InterpolationMethod method) {
    interpolationStrategy = createInterpolationStrategy(method);
    std::cout << "Interpolation method changed to: " << interpolationStrategy->getName() << std::endl;
}

std::string BladeGeometryInterpolator::getCurrentInterpolationMethod() const {
    return interpolationStrategy->getName();
}

// Generate interpolated blade sections for all geometry points
std::vector<InterpolatedBladeSection> BladeGeometryInterpolator::BladeGeometryInterpolator::interpolateAllSections() const {
    std::vector<InterpolatedBladeSection> interpolatedSections;

    for (const auto& section : bladeGeometry.getRows()) {
        try {
            auto interpolatedSection = interpolateSection(section);
            interpolatedSections.push_back(std::move(interpolatedSection));
        }
        catch (const std::exception& e) {
            std::cout << "Warning: Failed to interpolate section at r="
                << section.bladeRadius << "m: " << e.what() << std::endl;
        }
    }

    return interpolatedSections;
}

// Interpolate a single blade section
InterpolatedBladeSection BladeGeometryInterpolator::interpolateSection(const BladeGeometrySection& section) const {
    // Select appropriate airfoil for this section
    const AirfoilGeometryData* selectedAirfoil = selectAirfoilForSection(section);

    // Create interpolated section
    InterpolatedBladeSection interpSection(section.bladeRadius, section.chord,
        section.twist, selectedAirfoil->getName());

    // Scale airfoil coordinates to blade chord
    const auto& airfoilCoords = selectedAirfoil->getCoordinates();
    for (const auto& coord : airfoilCoords) {
        // Scale by chord length
        double scaledX = coord.x * section.chord;
        double scaledY = coord.y * section.chord;
        interpSection.scaledCoordinates.emplace_back(scaledX, scaledY);

        // Also create transformed coordinates (scaled + rotated + positioned)
        auto transformedCoord = transformCoordinate(coord, section.chord,
            section.twist, section.bladeRadius);
        interpSection.transformedCoordinates.push_back(transformedCoord);
    }

    return interpSection;
}

// Generate blade surface points for 3D visualization/meshing
std::vector<std::vector<AirfoilCoordinate>> BladeGeometryInterpolator::generateBladeSurface() const {
    auto interpolatedSections = interpolateAllSections();

    std::vector<std::vector<AirfoilCoordinate>> bladeSurface;

    for (const auto& section : interpolatedSections) {
        bladeSurface.push_back(section.transformedCoordinates);
    }

    return bladeSurface;
}

// Calculate blade volume (simplified)
double BladeGeometryInterpolator::calculateBladeVolume() const {
    auto interpolatedSections = interpolateAllSections();

    double totalVolume = 0.0;

    for (size_t i = 0; i < interpolatedSections.size() - 1; ++i) {
        const auto& section1 = interpolatedSections[i];
        const auto& section2 = interpolatedSections[i + 1];

        // Calculate cross-sectional areas (simplified as chord * thickness)
        double area1 = section1.chord * section1.chord * 0.1; // Simplified
        double area2 = section2.chord * section2.chord * 0.1;

        double height = section2.radius - section1.radius;
        double segmentVolume = (area1 + area2) * height * 0.5; // Trapezoidal rule

        totalVolume += segmentVolume;
    }

    return totalVolume;
}

// Generate coordinates for specific span positions using selected interpolation method
InterpolatedBladeSection BladeGeometryInterpolator::interpolateAtRadius(double targetRadius) const {
    const auto& sections = bladeGeometry.getRows();

    // Extract radius and parameter vectors for interpolation
    std::vector<double> radii;
    std::vector<double> chords;
    std::vector<double> twists;
    std::vector<double> thicknesses;

    for (const auto& section : sections) {
        radii.push_back(section.bladeRadius);
        chords.push_back(section.chord);
        twists.push_back(section.twist);
        thicknesses.push_back(section.relativeThickness);
    }

    // Check bounds
    if (targetRadius < radii.front() || targetRadius > radii.back()) {
        std::cout << "Warning: Interpolating outside data range (r=" << targetRadius
            << "m, range: " << radii.front() << "-" << radii.back() << "m)" << std::endl;
    }

    // Use strategy pattern for interpolation
    try {
        double interpChord = interpolationStrategy->interpolate(radii, chords, targetRadius);
        double interpTwist = interpolationStrategy->interpolate(radii, twists, targetRadius);
        double interpThickness = interpolationStrategy->interpolate(radii, thicknesses, targetRadius);

        // Create temporary section for interpolation
        BladeGeometrySection interpBlade;
        interpBlade.bladeRadius = targetRadius;
        interpBlade.chord = interpChord;
        interpBlade.twist = interpTwist;
        interpBlade.relativeThickness = interpThickness;

        return interpolateSection(interpBlade);

    }
    catch (const std::exception& e) {
        std::cout << "Interpolation failed, falling back to nearest section: " << e.what() << std::endl;

        // Fallback to nearest section
        auto it = std::lower_bound(sections.begin(), sections.end(), targetRadius,
            [](const BladeGeometrySection& section, double radius) {
                return section.bladeRadius < radius;
            });

        if (it == sections.begin()) {
            return interpolateSection(*it);
        }
        if (it == sections.end()) {
            return interpolateSection(sections.back());
        }

        // Find closest
        auto it_upper = it;
        auto it_lower = it - 1;

        double dist_lower = std::abs(targetRadius - it_lower->bladeRadius);
        double dist_upper = std::abs(targetRadius - it_upper->bladeRadius);

        return interpolateSection(dist_lower < dist_upper ? *it_lower : *it_upper);
    }
}

// Generate dense blade sections using interpolation
std::vector<InterpolatedBladeSection> BladeGeometryInterpolator::generateDenseBladeSections(size_t numSections) const {
    const auto& sections = bladeGeometry.getRows();

    if (sections.empty()) {
        throw std::runtime_error("No blade geometry data available");
    }

    std::vector<InterpolatedBladeSection> denseSections;

    double minRadius = sections.front().bladeRadius;
    double maxRadius = sections.back().bladeRadius;

    for (size_t i = 0; i < numSections; ++i) {
        double factor = static_cast<double>(i) / (numSections - 1);
        double radius = minRadius + factor * (maxRadius - minRadius);

        try {
            auto section = interpolateAtRadius(radius);
            denseSections.push_back(std::move(section));
        }
        catch (const std::exception& e) {
            std::cout << "Skipping section at r=" << radius << "m: " << e.what() << std::endl;
        }
    }

    return denseSections;
}

// Compare different interpolation methods
void BladeGeometryInterpolator::compareInterpolationMethods(double targetRadius) const {
    std::cout << "\n=== Interpolation Method Comparison at r=" << targetRadius << "m ===" << std::endl;

    std::vector<InterpolationMethod> methods = {
        InterpolationMethod::LINEAR,
        InterpolationMethod::CUBIC_SPLINE,
        InterpolationMethod::AKIMA_SPLINE,
        InterpolationMethod::MONOTONIC_CUBIC_SPLINE
    };

    const auto& sections = bladeGeometry.getRows();
    std::vector<double> radii;
    std::vector<double> chords;
    std::vector<double> twists;

    for (const auto& section : sections) {
        radii.push_back(section.bladeRadius);
        chords.push_back(section.chord);
        twists.push_back(section.twist);
    }

    for (auto method : methods) {
        auto strategy = createInterpolationStrategy(method);

        try {
            double interpChord = strategy->interpolate(radii, chords, targetRadius);
            double interpTwist = strategy->interpolate(radii, twists, targetRadius);

            std::cout << "  " << strategy->getName() << ": "
                << "Chord=" << interpChord << "m, "
                << "Twist=" << interpTwist << "°" << std::endl;

        }
        catch (const std::exception& e) {
            std::cout << "  " << strategy->getName() << ": Failed - " << e.what() << std::endl;
        }
    }
}

// Export blade geometry for CAD/CFD (simplified format)
void BladeGeometryInterpolator::exportBladeGeometry(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot create blade geometry export file: " + filename);
    }

    auto interpolatedSections = interpolateAllSections();

    file << "# Interpolated Blade Geometry Export\n";
    file << "# Radius[m] Chord[m] Twist[deg] Airfoil Points\n";

    for (const auto& section : interpolatedSections) {
        file << "SECTION " << section.radius << " " << section.chord
            << " " << section.twist << " " << section.airfoilName << "\n";

        // Export transformed coordinates
        for (const auto& coord : section.transformedCoordinates) {
            file << "POINT " << coord.x << " " << coord.y << "\n";
        }
        file << "END_SECTION\n";
    }

    file.close();
    std::cout << "Blade geometry exported to: " << filename << std::endl;
}
