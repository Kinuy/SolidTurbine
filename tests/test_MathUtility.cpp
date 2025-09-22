#include <gtest/gtest.h>

#include "../src/MathUtility.h"  // Reference main project
#include "../src/SquareMatrix.h"  // Reference main project
#include "../src/MathUtility.cpp" // Needs to be included if core project is build as Application (.exe) and not static library (.lib)

TEST(MathUtilityTest, basicLinearInterpolation_should_equal_given_number) {
    EXPECT_EQ(MathUtility::basicLinearInterpolation(2, 1, 3, 2, 4), 3); // Result should be 3
}

TEST(MathUtilityTest, basicLinearInterpolation_should_not_equal_given_number) {
    EXPECT_NE(MathUtility::basicLinearInterpolation(2, 1, 3, 2, 4), 5); // Result should not be 5

}

TEST(MathUtilityTest, matrixMultiplikation_should_equal_given_matrix_result) {
    //GIVEN
	SquareMatrix<double> m1({ 1, 2, 3, 4, 5, 6, 7, 8, 9 });
	SquareMatrix<double> m2({ 9, 8, 7, 6, 5, 4, 3, 2, 1 });
	SquareMatrix<double> result({ 30, 24, 18, 84, 69, 54, 138, 114, 90 });
	//WHEN
	EXPECT_EQ(MathUtility::matrixMultiplikation(m1, m2),result);
	//THEN
	// Check if the multiplication is correct
	// The expected result is calculated manually or using a calculator
	// Result should be:
	// 30, 24, 18
	// 84, 69, 54
}