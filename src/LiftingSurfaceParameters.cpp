
#include "LiftingSurfaceParameters.h"

int    LiftingSurfaceParameters::linearSolverMaxIterations = 20000;

double LiftingSurfaceParameters::linearSolverTolerance = std::numeric_limits<double>::epsilon();

bool   LiftingSurfaceParameters::unsteadyBernoulli = true;

bool   LiftingSurfaceParameters::convectWake = true;

bool   LiftingSurfaceParameters::wakeEmissionFollowBisector = true;

double LiftingSurfaceParameters::wakeEmissionDistanceFactor = 0.25;

double LiftingSurfaceParameters::wakeVortexCoreRadius = 0.0;

double LiftingSurfaceParameters::staticWakeLength = 100.0;

double LiftingSurfaceParameters::zeroThreshold = std::numeric_limits<double>::epsilon();


double LiftingSurfaceParameters::rlCollocationPointDelta = 1e-12;

double LiftingSurfaceParameters::rlInterpolationLayerThickness = 0.0;

int    LiftingSurfaceParameters::rlMaxBoundaryLayerIterations = 100;

double LiftingSurfaceParameters::rlBoundaryLayerIterationTolerance = std::numeric_limits<double>::epsilon();


// parameters for ramasamy leishman model

double LiftingSurfaceParameters::rlFluidKinematicViscosity = 15.68e-6;

double LiftingSurfaceParameters::rlInitialVortexCoreRadius = 0.07;

double LiftingSurfaceParameters::rlMinVortexCoreRadius = 0.07;

double LiftingSurfaceParameters::rlLambsConstant = 1.25643;

double LiftingSurfaceParameters::rlAPrime = 6.5e-5;



LiftingSurfaceParameters::ramasamy_leishman_data_row LiftingSurfaceParameters::ramasamy_leishman_data[12] = { {    1, 1.0000, 1.2560, 0.0000, 0.00000, 0.0000},
                                                                 {  100, 1.0000, 1.2515, 0.0000, 0.00000, 0.0000},
                                                                 { 1000, 1.0000, 1.2328, 0.0000, 0.00000, 0.0000},
                                                                 {10000, 0.8247, 1.2073, 0.1753, 0.02630, 0.0000},
                                                                 {2.5e4, 0.5933, 1.3480, 0.2678, 0.01870, 0.2070},
                                                                 {4.8e4, 0.4602, 1.3660, 0.3800, 0.01380, 0.1674},
                                                                 {7.5e4, 0.3574, 1.3995, 0.4840, 0.01300, 0.1636},
                                                                 {  1e5, 0.3021, 1.4219, 0.5448, 0.01220, 0.1624},
                                                                 {2.5e5, 0.1838, 1.4563, 0.6854, 0.00830, 0.1412},
                                                                 {  5e5, 0.1386, 1.4285, 0.7432, 0.00580, 0.1144},
                                                                 {7.5e5, 0.1011, 1.4462, 0.7995, 0.00480, 0.1078},
                                                                 {  1e6, 0.0792, 1.4716, 0.8352, 0.00420, 0.1077} };
