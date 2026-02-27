#include "MathUtility.h"
#include <stdexcept>
#include <numbers>

double MathUtility::getOneOver4Pi()
{
	return 1.0 / (4 * std::numbers::pi);
}

// Simple basic linear interpolation
double MathUtility::basicLinearInterpolation(
	double const& x,
	double const& x1,
	double const& x2,
	double const& y1,
	double const& y2)
{
	return (y1 + ((y2 - y1) * ((x - x1) / (x2 - x1))));
}

// Linear interpolation for a vector of values
double MathUtility::linearInterpolation(
	double target,
	std::vector< double> const& values,
	std::vector< double> const& data)
{
	double v0 = 0, v1 = 0;
	double data0 = 0, data1 = 0;
	if (values.size() == 1) {
		return data[0];
	}
	if (target <= values.front()) {
		target = values.front();
	}
	if (target > values.back()) {
		target = values.back();
	}
	for (std::size_t i = 0; i < values.size() - 1; ++i) {
		if (target <= values[i + 1]) {
			v0 = values[i];
			v1 = values[i + 1];
			data0 = data[i];
			data1 = data[i + 1];
			break;
		}
	}
	return basicLinearInterpolation(target, v0, v1, data0, data1);
}

// Bilinear interpolation for a 2D grid of data
double MathUtility::biLinearInterpolation(
	double target_mach,
	double const& target_alpha,
	std::vector< double> const& machs,
	std::vector< std::vector< double>> const& angles,
	std::vector< std::vector< double>> const& data)
{
	if (machs.size() == 1) {
		return linearInterpolation(target_alpha, angles[0], data[0]);
	}
	if (target_mach <= machs.front()) {
		target_mach = machs.front();
	}
	if (target_mach > machs.back()) {
		target_mach = machs.back();
	}
	std::size_t mach_block = 0;
	double data0 = 0;
	double data1 = 0;
	for (std::size_t i = 0; i < machs.size() - 1; ++i) {
		if (target_mach <= machs[i + 1]) {
			mach_block = i;
			data0 = linearInterpolation(target_alpha, angles[i], data[i]);
			data1 = linearInterpolation(target_alpha, angles[i + 1], data[i + 1]);
			break;
		}
	}
	return basicLinearInterpolation(target_mach, machs[mach_block], machs[mach_block + 1], data0, data1);
}

// Trilinear interpolation for a 3D grid of data
double MathUtility::triLinearInterpolation(
	double target_reynolds,
	double const& target_mach,
	double const& target_alpha,
	std::vector<double> const& reynolds,
	std::vector< std::vector<double> > const& machs,
	std::vector< std::vector< std::vector<double>>> const& angles,
	std::vector< std::vector< std::vector<double>>> const& data)
{
	if (reynolds.size() == 1) {
		return biLinearInterpolation(target_mach, target_alpha, machs[0], angles[0], data[0]);
	}
	if (target_reynolds <= reynolds.front()) {
		target_reynolds = reynolds.front();
	}
	if (target_reynolds > reynolds.back()) {
		target_reynolds = reynolds.back();
	}
	std::size_t re_block = 0;
	double data0 = 0;
	double data1 = 0;
	for (std::size_t i = 0; i < reynolds.size() - 1; ++i) {
		if (target_reynolds <= reynolds[i + 1]) {
			re_block = i;
			data0 = biLinearInterpolation(target_mach, target_alpha, machs[re_block], angles[re_block], data[re_block]);
			data1 = biLinearInterpolation(target_mach, target_alpha, machs[re_block + 1], angles[re_block + 1], data[re_block + 1]);
			break;
		}
	}
	return basicLinearInterpolation(target_reynolds, reynolds[re_block], reynolds[re_block + 1], data0, data1);
}

// TODO: delete this method later
SquareMatrix<double> MathUtility::matrixMultiplikation(SquareMatrix<double> const& m1, SquareMatrix<double> const& m2) {

	int dim = m1.getDim();
	// destination data is mc:
	std::vector<double> mc(dim * dim);
	for (std::size_t i = 0; i < mc.size(); ++i) {
		mc[i] = 0;
	}

	//calculation

	//Z1: [0]	[1]	[2]
	//Z2: [3]	[4]	[5]
	//Z3: [6]	[7]	[8]

	//Z1
	mc[0] = m1[0] * m2[0] + m1[1] * m2[3] + m1[2] * m2[6];
	mc[1] = m1[0] * m2[1] + m1[1] * m2[4] + m1[2] * m2[7];
	mc[2] = m1[0] * m2[2] + m1[1] * m2[5] + m1[2] * m2[8];
	//Z2
	mc[3] = m1[3] * m2[0] + m1[4] * m2[3] + m1[5] * m2[6];
	mc[4] = m1[3] * m2[1] + m1[4] * m2[4] + m1[5] * m2[7];
	mc[5] = m1[3] * m2[2] + m1[4] * m2[5] + m1[5] * m2[8];
	//Z.3
	mc[6] = m1[6] * m2[0] + m1[7] * m2[3] + m1[8] * m2[6];
	mc[7] = m1[6] * m2[1] + m1[7] * m2[4] + m1[8] * m2[7];
	mc[8] = m1[6] * m2[2] + m1[7] * m2[5] + m1[8] * m2[8];



	SquareMatrix<double> ret_m(m1.getDim());
	for (std::size_t i = 0; i < mc.size(); ++i) {
		ret_m[i] = mc[i];
	}
	return ret_m;
}

// TODO: delete this method later
void MathUtility::rotateVector3d(
	SquareMatrix<double> const& rotationMatrix,
	Vector3d<double>* targetVector3D) {

	// Put vector in RHS of a the matrix:
	SquareMatrix<double> matrix(3);
	matrix[2] = targetVector3D->x();
	matrix[5] = targetVector3D->y();
	matrix[8] = targetVector3D->z();

	SquareMatrix<double> matrixProduct = matrixMultiplikation(rotationMatrix, matrix);

	// overwrite input vector with the rotation result:
	(*targetVector3D)[0] = matrixProduct[2];
	(*targetVector3D)[1] = matrixProduct[5];
	(*targetVector3D)[2] = matrixProduct[8];
}

// TODO: delete this method later
// Create a rotation matrix for a clockwise rotation, looking in positive x-axis, around the X-axis -> axis vertical on rotor plane
SquareMatrix<double> MathUtility::createMatrixClockwiseRotationAroundXAxis(const double rotationAngle) {
	SquareMatrix<double> matrix(3);
	std::array<double, 9> matrixXRotation = {
		1, 0, 0,
		0, cos(rotationAngle), sin(rotationAngle),
		0,-1.0 * sin(rotationAngle),cos(rotationAngle) };

	for (std::size_t i = 0; i < matrixXRotation.size(); ++i) {
		matrix[i] = matrixXRotation[i];
	}
	return matrix;
}

// Direct matrix-vector multiplication
Vector3d<double> MathUtility::multiplyMatrixVector(const SquareMatrix<double>& matrix, const Vector3d<double>& vec) {
	Vector3d<double> vec3d;
	vec3d = {
		matrix[0] * vec.x() + matrix[1] * vec.y() + matrix[2] * vec.z(),
		matrix[3] * vec.x() + matrix[4] * vec.y() + matrix[5] * vec.z(),
		matrix[6] * vec.x() + matrix[7] * vec.y() + matrix[8] * vec.z()
	};

	return vec3d;
}

// Simple matrix multiplication with bounds checking
SquareMatrix<double> MathUtility::multiplyMatrices(const SquareMatrix<double>& m1, const SquareMatrix<double>& m2) {
	if (m1.getDim() != m2.getDim()) {
		throw std::invalid_argument("Matrix dimensions must match");
	}

	const int dim = m1.getDim();
	SquareMatrix<double> result(dim);

	for (int i = 0; i < dim; ++i) {
		for (int j = 0; j < dim; ++j) {
			double sum = 0.0;
			for (int k = 0; k < dim; ++k) {
				sum += m1[i * dim + k] * m2[k * dim + j];
			}
			result[i * dim + j] = sum;
		}
	}
	return result;
}

// Create X-rotation matrix directly with clockwise rotation, looking in positive x-axis, around the X-axis -> axis vertical on rotor plane
SquareMatrix<double> MathUtility::createXRotationMatrix(const double psi) {
	SquareMatrix<double> matrix(3);
	const double c = cos(psi);
	const double s = sin(psi);

	// Row-major order: [0,1,2], [3,4,5], [6,7,8]
	matrix[0] = 1;  matrix[1] = 0;  matrix[2] = 0;
	matrix[3] = 0;  matrix[4] = c;  matrix[5] = s;
	matrix[6] = 0;  matrix[7] = -s; matrix[8] = c;

	return matrix;
}

// Simplified coordinate conversion from global to local coordinates
Vector3d<double> MathUtility::convertGlobalToLocal(
	const Vector3d<double>& vec, 
	const double psi,
	const SquareMatrix<double>& a34, 
	const SquareMatrix<double>& a12) {

	// Create transformation matrix: a34 * createXRotationMatrix(psi) * a12
	SquareMatrix<double> a23 = createXRotationMatrix(psi);
	SquareMatrix<double> a13 = multiplyMatrices(a23, a12);
	SquareMatrix<double> a14 = multiplyMatrices(a34, a13);

	// Apply transformation to vector
	return multiplyMatrixVector(a14, vec);
}

// Create reverse X-rotation matrix 
SquareMatrix<double> MathUtility::createInverseXRotationMatrix(double psi, const SquareMatrix<double>& a43) {
	// Since transpose(X_rotation(psi)) = X_rotation(-psi), just use negative angle
	SquareMatrix<double> a32 = createXRotationMatrix(-psi);
	return multiplyMatrices(a32, a43);
}