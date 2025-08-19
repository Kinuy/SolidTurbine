#pragma once

#include <vector>

/**
 * @brief Template class for square matrix operations with generic numeric types
 *
 * SquareMatrix provides efficient storage and basic operations for square matrices
 * using row-major order in a contiguous std::vector. Supports any numeric type
 * and common matrix operations like transposition and identity matrix creation.
 *
 * ## Storage Format
 * - **Row-Major Order**: Element (i,j) stored at index [i*dim + j]
 * - **Contiguous Memory**: Uses std::vector for cache-friendly access
 * - **Zero Initialization**: All constructors create zero-filled matrices
 *
 * ## Key Features
 * - Flexible sizing (default 3x3 or custom dimensions)
 * - In-place transpose and identity matrix operations
 * - Element access via subscript operator
 * - Equality comparison support
 *
 * @tparam T Numeric type for matrix elements (double, float, int, etc.)
 *
 * @note Matrix dimensions are immutable after construction
 * @note No bounds checking on operator[] - use carefully
 *
 * @example
 * ```cpp
 * // Different construction methods
 * SquareMatrix<double> defaultMat;           // 3x3 zeros
 * SquareMatrix<double> customMat(5);         // 5x5 zeros
 * SquareMatrix<double> valueMat(1,0,0, 0,1,0, 0,0,1); // 3x3 identity
 *
 * // Basic operations
 * defaultMat.makeIdentity();
 * defaultMat.transpose();
 * double element = defaultMat[4]; // Access element at index 4
 * ```
 */
template <typename T>
class SquareMatrix {

private:

    /**
     * @brief Matrix dimension (number of rows/columns)
     */
    unsigned int dim;

    /**
     * @brief Matrix data in row-major order
     *
     * Elements stored as [row0_col0, row0_col1, ..., row0_colN,
     * row1_col0, row1_col1, ..., rowN_colN]
     */
    std::vector<T> data;

public:

    /**
     * @brief Default constructor creating a 3x3 zero matrix
     */
    SquareMatrix()
        : dim(3) {
        data.resize(dim * dim);
        for (std::size_t i = 0; i < data.size(); ++i) {
            data[i] = 0;
        }
    };

    /**
     * @brief Constructor creating an NxN zero matrix
     * @param dim Matrix dimension (number of rows and columns)
     */
    SquareMatrix(unsigned int dim)
        : dim(dim) {
        data.resize(dim * dim);
        for (std::size_t i = 0; i < data.size(); ++i) {
            data[i] = 0;
        }
    };

    /**
     * @brief Constructor creating a 3x3 matrix with specified values
     * @param r1c1,r1c2,r1c3 First row elements
     * @param r2c1,r2c2,r2c3 Second row elements
     * @param r3c1,r3c2,r3c3 Third row elements
     */
    SquareMatrix(T r1c1,
        T r1c2,
        T r1c3,
        T r2c1,
        T r2c2,
        T r2c3,
        T r3c1,
        T r3c2,
        T r3c3)
        : dim(3) {
        data.resize(dim * dim);
        data[0] = r1c1;
        data[1] = r1c2;
        data[2] = r1c3;
        data[3] = r2c1;
        data[4] = r2c2;
        data[5] = r2c3;
        data[6] = r3c1;
        data[7] = r3c2;
        data[8] = r3c3;
    }

    /**
     * @brief Converts matrix to identity matrix in-place
     * Sets diagonal elements to 1, all others to 0
     */
    void makeIdentity() {
        for (std::size_t i = 0; i < dim; ++i) {
            for (std::size_t j = 0; j < dim; ++j) {
                data[i * dim + j] = 0;
                if (i == j) {
                    data[i * dim + j] = 1;
                }
            }
        }
    };

    /**
     * @brief Mutable element access by linear index
     * @param i Linear index in row-major order
     * @return Reference to matrix element
     * @warning No bounds checking performed
     */
    T& operator[] (std::size_t const& i) {
        return data[i];
    }

    /**
     * @brief Const element access by linear index
     * @param i Linear index in row-major order
     * @return Copy of matrix element
     */
    T operator[] (std::size_t const& i) const {
        return data[i];
    }

    /**
     * @brief Equality comparison operator
     * @param other Matrix to compare with
     * @return true if dimensions and all elements are equal
     */
    bool operator==(const SquareMatrix<T>& other) const {
        if (dim != other.dim) return false;
        return data == other.data;  // std::vector has == operator
    }

    /**
     * @brief Transposes the matrix in-place
     * Swaps rows and columns: element (i,j) becomes (j,i)
     */
    void transpose() {
        std::vector<T> tempData;
        for (std::size_t i = 0; i < dim; ++i) {
            for (std::size_t j = 0; j < dim; ++j) {
                tempData.push_back(data[j * dim + i]);
            }
        }
        data = tempData;
    }

    /**
     * @brief Gets the matrix dimension
     * @return Number of rows (equals number of columns)
     */
    unsigned int getDim() const { return dim; };

    /**
     * @brief Gets a copy of the internal data vector
     * @return Copy of matrix data in row-major order
     */
    std::vector<T> getData() const { return data; };

};
