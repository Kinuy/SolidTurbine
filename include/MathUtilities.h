#ifndef CC_MATHUTILITIES_H_
#define CC_MATHUTILITIES_H_

#include <array>
#include <cmath>
#include <functional>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <vector>

/// @file mathutilities.h
/// @brief Lightweight linear algebra types and mathematical utility functions.
///
/// Provides:
/// - @ref WVPMUtilities::Vec3D        — a 3-component vector template
/// - @ref WVPMUtilities::SquareMatrix — a row-major N×N matrix template
/// - Free functions for matrix multiplication, interpolation, root finding,
///   and special functions (Gamma, LogGamma).
///
/// No third-party linear algebra library is required; all types and algorithms
/// are self-contained to simplify cross-platform builds.

namespace WVPMUtilities {

// ---------------------------------------------------------------------------
/// @brief Templated 3-component Cartesian vector.
///
/// Supports element access via x()/y()/z() and the standard arithmetic
/// operators (+, -, +=, -=). Indexing via @c operator[] is also provided for
/// compatibility with rotation routines.
///
/// @tparam T Numeric value type (typically @c double or @c float).
// ---------------------------------------------------------------------------
template <typename T>
class Vec3D {
public:
    /// @brief Default-constructs a zero vector.
    Vec3D() = default;

    /// @brief Constructs a vector from three scalar components.
    /// @param a  X component.
    /// @param b  Y component.
    /// @param c  Z component.
    Vec3D(T a, T b, T c) : data_{a, b, c} {}

    /// @name Component accessors
    /// @{
    T x() const { return data_[0]; } ///< Returns the X component.
    T y() const { return data_[1]; } ///< Returns the Y component.
    T z() const { return data_[2]; } ///< Returns the Z component.
    /// @}

    /// @brief Mutable element access by index (0 = x, 1 = y, 2 = z).
    T& operator[](std::size_t i) { return data_[i]; }

    /// @brief Immutable element access by index (0 = x, 1 = y, 2 = z).
    T const& operator[](std::size_t i) const { return data_.at(i); }

    /// @brief Compound addition — adds @a rhs element-wise.
    Vec3D& operator+=(Vec3D const& rhs) {
        data_[0] += rhs.x(); data_[1] += rhs.y(); data_[2] += rhs.z();
        return *this;
    }

    /// @brief Compound subtraction — subtracts @a rhs element-wise.
    Vec3D& operator-=(Vec3D const& rhs) {
        data_[0] -= rhs.x(); data_[1] -= rhs.y(); data_[2] -= rhs.z();
        return *this;
    }

    /// @brief Returns the element-wise sum of this vector and @a rhs.
    Vec3D operator+(Vec3D const& rhs) const {
        return {x() + rhs.x(), y() + rhs.y(), z() + rhs.z()};
    }

    /// @brief Returns the element-wise difference of this vector and @a rhs.
    Vec3D operator-(Vec3D const& rhs) const {
        return {x() - rhs.x(), y() - rhs.y(), z() - rhs.z()};
    }

private:
    std::array<T, 3> data_{};
};

/// @brief Stream insertion operator for Vec3D.
/// @relates Vec3D
template <typename T>
std::ostream& operator<<(std::ostream& os, Vec3D<T> const& v) {
    return os << "x: " << v.x() << "   y: " << v.y() << "   z: " << v.z();
}

// ---------------------------------------------------------------------------
/// @brief Row-major N×N square matrix.
///
/// Elements are stored in a flat @c std::vector in row-major order:
/// element (row, col) maps to flat index @c row*dim+col.
///
/// Provides in-place transposition, identity initialisation, and both mutable
/// and immutable element access via @c operator[].
///
/// @tparam T Numeric value type (typically @c double).
// ---------------------------------------------------------------------------
template <typename T>
class SquareMatrix {
public:
    /// @brief Constructs a 3×3 zero matrix.
    SquareMatrix() : SquareMatrix(3) {}

    /// @brief Constructs an N×N zero matrix.
    /// @param dim  Side length of the square matrix.
    explicit SquareMatrix(unsigned int dim)
        : dim_(dim), data_(dim * dim, T{}) {}

    /// @brief Convenience constructor for a 3×3 matrix with explicit values.
    ///
    /// Parameters are supplied in row-major order:
    /// @code
    /// SquareMatrix m(r1c1, r1c2, r1c3,
    ///                r2c1, r2c2, r2c3,
    ///                r3c1, r3c2, r3c3);
    /// @endcode
    SquareMatrix(T r1c1, T r1c2, T r1c3,
                 T r2c1, T r2c2, T r2c3,
                 T r3c1, T r3c2, T r3c3)
        : dim_(3)
        , data_{r1c1, r1c2, r1c3,
                r2c1, r2c2, r2c3,
                r3c1, r3c2, r3c3}
    {}

    /// @brief Mutable element access by flat row-major index.
    T&       operator[](std::size_t i)       { return data_[i]; }

    /// @brief Immutable element access by flat row-major index.
    T const& operator[](std::size_t i) const { return data_[i]; }

    /// @brief Returns the side length N of the N×N matrix.
    unsigned int dim() const { return dim_; }

    /// @brief Returns a copy of the underlying flat data array.
    std::vector<T> data() const { return data_; }

    /// @brief Sets this matrix to the identity matrix in-place.
    void MakeIdentity() {
        std::fill(data_.begin(), data_.end(), T{});
        for (unsigned int i = 0; i < dim_; ++i) {
            data_[i * dim_ + i] = T{1};
        }
    }

    /// @brief Transposes the matrix in-place.
    void Transpose() {
        std::vector<T> tmp(dim_ * dim_);
        for (unsigned int i = 0; i < dim_; ++i) {
            for (unsigned int j = 0; j < dim_; ++j) {
                tmp[i * dim_ + j] = data_[j * dim_ + i];
            }
        }
        data_ = std::move(tmp);
    }

private:
    unsigned int   dim_;
    std::vector<T> data_;
};

// ---------------------------------------------------------------------------
/// @name Utility templates
/// @{
// ---------------------------------------------------------------------------

/// @brief Returns @a N evenly spaced values over the closed interval [@a a, @a b].
/// @tparam T  Floating-point type.
/// @param  a  Start of the interval (inclusive).
/// @param  b  End of the interval (inclusive).
/// @param  N  Number of points; must be ≥ 2.
/// @return    Vector of @a N values uniformly distributed from @a a to @a b.
template <typename T>
std::vector<T> linspace(T a, T b, std::size_t N) {
    std::vector<T> xs(N);
    const T step = (b - a) / static_cast<T>(N - 1);
    T val = a;
    for (auto& x : xs) { x = val; val += step; }
    return xs;
}

/// @brief Clamps @a n to the closed interval [@a lower, @a upper].
/// @tparam T  Any type supporting @c std::min and @c std::max.
/// @param  n      Value to clamp.
/// @param  lower  Minimum allowed value.
/// @param  upper  Maximum allowed value.
/// @return        @a lower if @a n < @a lower, @a upper if @a n > @a upper,
///                otherwise @a n.
template <typename T>
T clip(T const& n, T const& lower, T const& upper) {
    return std::max(lower, std::min(n, upper));
}

/// @}

// ---------------------------------------------------------------------------
/// @name Matrix operations
/// @{
// ---------------------------------------------------------------------------

/// @brief Multiplies two square matrices and returns the result.
///
/// Implements the standard O(N³) algorithm. The dimension is taken from @a m1.
///
/// @param m1  Left-hand operand.
/// @param m2  Right-hand operand (must have the same dimension as @a m1).
/// @return    Product @a m1 × @a m2 as a new matrix.
SquareMatrix<double> MatMult(SquareMatrix<double> const& m1,
                              SquareMatrix<double> const& m2);

/// @brief Rotates a Vec3D in-place using a 3×3 rotation matrix.
///
/// Computes the transformation:
/// @code
///   *target_vec = rotation_matrix * (*target_vec)
/// @endcode
/// and writes the result back to @a target_vec.
///
/// @param rotation_matrix  3×3 rotation matrix.
/// @param[in,out] target_vec  Vector to rotate; overwritten with the result.
void RotateVec3D(SquareMatrix<double> const& rotation_matrix,
                 Vec3D<double>* target_vec);

/// @}

// ---------------------------------------------------------------------------
/// @name Interpolation
/// @{
// ---------------------------------------------------------------------------

/// @brief Builds a sorted lookup table for use with @ref linear_interpolation.
///
/// @a x and @a y must be the same length. If @a x contains duplicates,
/// behaviour is implementation-defined (the last value for a given x is kept).
///
/// @param x  Independent variable samples.
/// @param y  Dependent variable samples corresponding to each element of @a x.
/// @return   A @c std::map<double,double> sorted by x.
std::map<double, double> fill_for_linear_interpolation(
    std::vector<double> const& x,
    std::vector<double> const& y);

/// @brief Linearly interpolates within a sorted (x, y) lookup table.
///
/// Returns the y value that corresponds to @a x by piecewise linear
/// interpolation. Values outside the table range are clamped to the
/// nearest endpoint rather than extrapolated.
///
/// @param x      Query point.
/// @param table  Sorted map produced by @ref fill_for_linear_interpolation.
/// @return       Interpolated y at @a x, clamped at the table boundaries.
double linear_interpolation(double x, std::map<double, double> const& table);

/// @brief Interpolates using a cubic spline (stub implementation).
///
/// @note Currently returns 1.0 unconditionally. Replace with a proper
///       cubic-spline solver when a suitable library is available.
///
/// @param target_x  Query point.
/// @param x         Monotonically increasing x-coordinates of known points.
/// @param y         Corresponding y-coordinates (same length as @a x).
/// @return          Interpolated y value at @a target_x.
double SplineForY(double target_x,
                  std::vector<double> const& x,
                  std::vector<double> const& y);

/// @}

// ---------------------------------------------------------------------------
/// @name Root finding
/// @{
// ---------------------------------------------------------------------------

/// @brief Finds a root of @a f in [@a lower_bound, @a upper_bound] using
///        Brent's method.
///
/// Brent's method guarantees convergence by combining bisection (safe but
/// slow), the secant method, and inverse quadratic interpolation (fast when
/// the iterates are well-behaved).
///
/// @pre  @a f(@a lower_bound) and @a f(@a upper_bound) must have opposite
///       signs (i.e. the root must be bracketed).
///
/// @param f            Continuous scalar function to find the root of.
/// @param lower_bound  Left endpoint of the bracketing interval.
/// @param upper_bound  Right endpoint of the bracketing interval.
/// @return             Approximate root of @a f within tolerance 1e-4,
///                     or 0 if the bracket condition is violated.
double brents_fun(std::function<double(double)> f,
                  double lower_bound,
                  double upper_bound);

/// @}

// ---------------------------------------------------------------------------
/// @name Special functions
/// @{
// ---------------------------------------------------------------------------

/// @brief Computes the Gamma function Γ(x) for positive @a x.
///
/// Uses a rational minimax approximation on the interval (1, 2) combined
/// with recurrence relations for arguments outside that range, and the
/// asymptotic series for large @a x (> 12).
///
/// @param x  Argument; must be positive. Non-positive integers are poles of
///           the Gamma function; this implementation returns a large value
///           near those points.
/// @return   Γ(x), or approximately @c DBL_MAX × 2 when @a x > 171.624
///           (overflow region).
double Gamma(double x);

/// @brief Computes the natural logarithm of the Gamma function, ln Γ(x).
///
/// Numerically superior to @c std::log(Gamma(x)) for large arguments, where
/// it uses the asymptotic Stirling series from Abramowitz & Stegun §6.1.41.
/// For @a x < 12 it falls back to @c std::log(|Γ(x)|).
///
/// @param x  Argument; must be strictly positive.
/// @return   ln Γ(x).
/// @throws   std::invalid_argument if @a x ≤ 0.
double LogGamma(double x);

/// @}

} // namespace WVPMUtilities

#endif // CC_MATHUTILITIES_H_
