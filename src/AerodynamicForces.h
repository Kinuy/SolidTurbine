#pragma once

// Aerodynamic Forces - responsible for storing blade section aerodynamic forces data
struct AerodynamicForces {

	double powerCoeffiCp;	// Power coefficient Cp [-]
	double thrustCoeffCt;	// Thrust coefficient Ct [-]
	double torqueCoeffCm;	// Torque coefficient Cm [-]
	double edgeForceFx;		// Edge force in x direction Fx [N]
	double flapForceFy;		// Flap force in y direction Fy [N]
	double Fz;				// Force in z direction Fz [N]
	double edgeMomentMx;	// Edge moment in x direction Mx [Nm]
	double flapMomentMy;	// Flap moment in y direction My [Nm]
	double Mz;				// Moment in z direction Mz [Nm]

	AerodynamicForces(
		const double powerCoeffiCp = 0.0, 
		const double thrustCoeffCt = 0.0, 
		const double torqueCoeffCm = 0.0, 
		const double edgeForceFx = 0.0, 
		const double flapForceFy = 0.0, 
		const double Fz = 0.0, 
		const double edgeMomentMx = 0.0, 
		const double flapMomentMy = 0.0, 
		const double Mz = 0.0)
		: 
		powerCoeffiCp(powerCoeffiCp), 
		thrustCoeffCt(thrustCoeffCt), 
		torqueCoeffCm(torqueCoeffCm), 
		edgeForceFx(edgeForceFx), 
		flapForceFy(flapForceFy), 
		Fz(Fz), 
		edgeMomentMx(edgeMomentMx), 
		flapMomentMy(flapMomentMy), 
		Mz(Mz) {
	}
};
