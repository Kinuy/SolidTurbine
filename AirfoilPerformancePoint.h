#pragma once

// Individual Airfoil Performance Data Structure
struct AirfoilPerformancePoint {
    double alpha;  // Angle of attack [degrees]
    double cl;     // Lift coefficient
    double cd;     // Drag coefficient  
    double cm;     // Moment coefficient (optional)

    AirfoilPerformancePoint(double alpha, double cl, double cd, double cm)
        : alpha(alpha), cl(cl), cd(cd), cm(cm) {
    }
};
