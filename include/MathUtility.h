#pragma once

#include <vector>
#include <array>
#include <cmath>

#include "Vector3d.h"
#include "SquareMatrix.h"

/**
 * @brief Static utility class providing mathematical operations and algorithms
 *
 * MathUtility provides a collection of static mathematical functions for
 * interpolation, matrix operations, coordinate transformations, and vector
 * calculations. Designed as a pure utility class that cannot be instantiated.
 *
 * ## Key Features
 * - **Interpolation**: Linear, bilinear, and trilinear interpolation methods
 * - **Matrix Operations**: Matrix multiplication, rotation matrices, transformations
 * - **Vector Operations**: 3D vector transformations and coordinate conversions
 * - **Static Interface**: All methods are static, no instantiation required
 *
 * ## Interpolation Capabilities
 * - 1D linear interpolation with boundary clamping
 * - 2D bilinear interpolation for aerodynamic data grids
 * - 3D trilinear interpolation for complex parameter spaces
 *
 * ## Matrix/Vector Operations
 * - 3x3 matrix multiplication and rotation matrices
 * - Coordinate system transformations
 * - Matrix-vector multiplication utilities
 *
 * @note This class cannot be instantiated - all methods are static
 * @note Some methods marked as deprecated have improved alternatives
 *
 * @example
 * ```cpp
 * // Linear interpolation
 * double result = MathUtility::linearInterpolation(2.5, xData, yData);
 *
 * // Matrix operations
 * auto rotMatrix = MathUtility::createXRotationMatrix(angle);
 * auto transformedVec = MathUtility::multiplyMatrixVector(matrix, vector);
 * ```
 */
class MathUtility
{
private:
	/**
	 * @brief Deleted default constructor - prevents instantiation
	 */
	MathUtility() = delete;

	/**
	 * @brief Deleted copy constructor - prevents copying
	 */
	MathUtility(const MathUtility &) = delete;

	/**
	 * @brief Deleted assignment operator - prevents assignment
	 */
	MathUtility &operator=(const MathUtility &) = delete;

public:
	/**
	 * @brief Basic linear interpolation between two points
	 * @param x Target value for interpolation
	 * @param x1,x2 X-coordinates of interpolation bounds
	 * @param y1,y2 Y-coordinates of interpolation bounds
	 * @return Interpolated y-value at x
	 */
	static double basicLinearInterpolation(double const &x, double const &x1, double const &x2, double const &y1, double const &y2);

	/**
	 * @brief Linear interpolation for vector-based data with boundary clamping
	 * @param target Target value for interpolation
	 * @param values Vector of x-coordinates (independent variable)
	 * @param data Vector of y-coordinates (dependent variable)
	 * @return Interpolated value, clamped to data bounds
	 * @note Returns single value if only one data point provided
	 */
	static double linearInterpolation(double target, std::vector<double> const &values, std::vector<double> const &data);

	/**
	 * @brief Bilinear interpolation for 2D data grid (Mach-angle interpolation)
	 * @param target_mach Target Mach number
	 * @param target_alpha Target angle of attack
	 * @param machs Vector of Mach numbers
	 * @param angles 2D vector of angle grids for each Mach
	 * @param data 2D vector of data values
	 * @return Interpolated value with boundary clamping
	 */
	static double biLinearInterpolation(
		double target_mach,
		double const &target_alpha,
		std::vector<double> const &machs,
		std::vector<std::vector<double>> const &angles,
		std::vector<std::vector<double>> const &data);

	/**
	 * @brief Trilinear interpolation for 3D data grid (Reynolds-Mach-angle)
	 * @param target_reynolds Target Reynolds number
	 * @param target_mach Target Mach number
	 * @param target_alpha Target angle of attack
	 * @param reynolds Vector of Reynolds numbers
	 * @param machs 2D vector of Mach grids
	 * @param angles 3D vector of angle grids
	 * @param data 3D vector of data values
	 * @return Interpolated value with boundary clamping
	 */
	static double triLinearInterpolation(
		double target_reynolds,
		double const &target_mach,
		double const &target_alpha,
		std::vector<double> const &reynolds,
		std::vector<std::vector<double>> const &machs,
		std::vector<std::vector<std::vector<double>>> const &angles,
		std::vector<std::vector<std::vector<double>>> const &data);

	/**
	 * @brief Matrix multiplication for 3x3 matrices
	 * @param m1,m2 Square matrices to multiply
	 * @return Product matrix m1 * m2
	 * @deprecated Use multiplyMatrices() for improved implementation
	 */
	static SquareMatrix<double> matrixMultiplikation(SquareMatrix<double> const &m1, SquareMatrix<double> const &m2);

	/**
	 * @brief Rotates a 3D vector using rotation matrix
	 * @param rotationMatrix 3x3 rotation matrix
	 * @param targetVector3D Vector to rotate (modified in-place)
	 * @deprecated Use multiplyMatrixVector() for cleaner implementation
	 */
	static void rotateVector3d(SquareMatrix<double> const &rotationMatrix, Vector3d<double> *targetVector3D);

	/**
	 * @brief Creates clockwise X-axis rotation matrix
	 * @param rotationAngle Rotation angle in radians
	 * @return 3x3 rotation matrix for X-axis rotation
	 * @deprecated Use createXRotationMatrix() for improved version
	 */
	static SquareMatrix<double> createMatrixClockwiseRotationAroundXAxis(const double rotationAngle);

	/**
	 * @brief Direct matrix-vector multiplication
	 * @param matrix 3x3 matrix
	 * @param vec 3D vector
	 * @return Transformed vector (matrix * vec)
	 */
	static Vector3d<double> multiplyMatrixVector(const SquareMatrix<double> &matrix, const Vector3d<double> &vec);

	/**
	 * @brief General matrix multiplication with dimension checking
	 * @param m1,m2 Square matrices to multiply
	 * @return Product matrix m1 * m2
	 * @throws std::invalid_argument if matrix dimensions don't match
	 */
	static SquareMatrix<double> multiplyMatrices(const SquareMatrix<double> &m1, const SquareMatrix<double> &m2);

	/**
	 * @brief Creates X-axis rotation matrix for given angle
	 * @param psi Rotation angle in radians (clockwise when looking along +X axis)
	 * @return 3x3 rotation matrix
	 */
	static SquareMatrix<double> createXRotationMatrix(const double psi);

	/**
	 * @brief Converts vector from global to local coordinate system
	 * @param vec Vector in global coordinates
	 * @param psi X-axis rotation angle in radians
	 * @param a34,a12 Transformation matrices for coordinate system conversion
	 * @return Vector in local coordinate system
	 */
	static Vector3d<double> convertGlobalToLocal(const Vector3d<double> &vec, const double psi, const SquareMatrix<double> &a34, const SquareMatrix<double> &a12);

	/**
	 * @brief Creates inverse X-rotation matrix combined with transformation
	 * @param psi Rotation angle in radians
	 * @param a43 Transformation matrix
	 * @return Combined inverse rotation and transformation matrix
	 */
	static SquareMatrix<double> createInverseXRotationMatrix(double psi, const SquareMatrix<double> &a43);

	/**
	 * @brief Helper method to return constant 1/(4*?)
	 * @return Constant value of 1/(4*?)
	 */
	static double getOneOver4Pi();
};
