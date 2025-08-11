#pragma once

#include <vector>
#include <array>
#include <cmath>

#include "Vector3d.h"
#include "SquareMatrix.h"

// MathUtility - responsible for all math related calculations
class MathUtility
{
private:

	// No default constructor
	MathUtility() = delete;

	// No default constructor
	MathUtility(const MathUtility&) = delete;

	// No assignment operator
	MathUtility& operator=(const MathUtility&) = delete;

public:

	static double basicLinearInterpolation(double const& x, double const& x1, double const& x2, double const& y1, double const& y2);
	
	static double linearInterpolation(double target, std::vector<double> const& values, std::vector<double> const& data);
	
	static double biLinearInterpolation(
		double target_mach, 
		double const& target_alpha, 
		std::vector<double> const& machs, 
		std::vector<std::vector<double>> const& angles, 
		std::vector<std::vector<double>> const& data);
	
	static double triLinearInterpolation(
		double target_reynolds, 
		double const& target_mach, 
		double const& target_alpha, 
		std::vector<double> const& reynolds, 
		std::vector<std::vector<double>> const& machs, 
		std::vector<std::vector<std::vector<double>>> const& angles, 
		std::vector<std::vector<std::vector<double>>> const& data);

	static SquareMatrix<double> matrixMultiplikation(SquareMatrix<double> const& m1, SquareMatrix<double> const& m2);

	static void rotateVector3d(SquareMatrix<double> const& rotationMatrix, Vector3d<double>* targetVector3D);

	static SquareMatrix<double> createMatrixClockwiseRotationAroundXAxis(const double rotationAngle);

	static Vector3d<double> multiplyMatrixVector(const SquareMatrix<double>& matrix, const Vector3d<double>& vec);

	static SquareMatrix<double> multiplyMatrices(const SquareMatrix<double>& m1, const SquareMatrix<double>& m2);

	static SquareMatrix<double> createXRotationMatrix(const double psi);

	static Vector3d<double> convertGlobalToLocal(const Vector3d<double>& vec, const double psi, const SquareMatrix<double>& a34, const SquareMatrix<double>& a12);

	static SquareMatrix<double> createInverseXRotationMatrix(double psi, const SquareMatrix<double>& a43);

};


