#pragma once

#include <array>

/**
 * @brief Template class for 3D vector operations with generic numeric types
 *
 * Vector3d provides a lightweight, efficient implementation of 3-dimensional vectors
 * with support for common mathematical operations. The template design allows usage
 * with any numeric type (int, float, double, custom numeric types) while maintaining
 * type safety and performance.
 *
 * ## Mathematical Foundation
 *
 * A 3D vector represents a point in 3-dimensional Euclidean space or a direction
 * and magnitude in 3D space. This implementation stores three components (x, y, z)
 * and provides standard vector arithmetic operations.
 *
 * ## Template Design
 *
 * The template parameter T allows for flexible numeric types:
 * - **Floating-point**: Vector3d<double>, Vector3d<float> for continuous mathematics
 * - **Integer**: Vector3d<int>, Vector3d<long> for discrete coordinates
 * - **Custom types**: Any type supporting arithmetic operations (+, -, +=, -=)
 *
 * ## Memory Layout
 *
 * Uses std::array<T, 3> for efficient, stack-allocated storage with guaranteed
 * contiguous memory layout. This provides:
 * - Cache-friendly memory access patterns
 * - Minimal memory overhead (exactly 3*sizeof(T) bytes)
 * - Compatibility with C-style arrays when needed
 *
 * ## Use Cases
 *
 * Common applications include:
 * - 3D graphics and rendering (positions, directions, normals)
 * - Physics simulations (forces, velocities, accelerations)
 * - Engineering calculations (coordinates, displacements)
 * - Game development (player positions, movement vectors)
 * - Scientific computing (3D data analysis, spatial algorithms)
 *
 * @tparam T Numeric type for vector components. Must support arithmetic operations
 *           (+, -, +=, -=) and copy construction. Examples: double, float, int
 *
 * @note This class does not provide normalization, dot product, cross product,
 *       or magnitude operations. These can be implemented as free functions
 *       or added as additional methods if needed.
 *
 * @note The class provides both const and non-const versions of operator[]
 *       for flexible element access with appropriate safety guarantees.
 *
 * @thread_safety This class is not thread-safe. External synchronization is
 *                required for concurrent access to the same Vector3d instance.
 *
 * @example
 * ```cpp
 * // Basic usage with different numeric types
 * Vector3d<double> position(1.5, 2.3, -0.7);
 * Vector3d<int> gridCoord(10, 20, 5);
 * Vector3d<float> velocity(0.0f, -9.8f, 0.0f);
 *
 * // Vector arithmetic
 * Vector3d<double> displacement = position + velocity;
 * position += velocity; // In-place addition
 *
 * // Component access
 * double x_coord = position.x();
 * position[1] = 5.0; // Modify y component
 * ```
 *
 * @example
 * ```cpp
 * // Physics simulation example
 * Vector3d<double> position(0.0, 10.0, 0.0);
 * Vector3d<double> velocity(5.0, 0.0, 0.0);
 * Vector3d<double> gravity(0.0, -9.81, 0.0);
 *
 * // Simple physics integration
 * double dt = 0.016; // 60 FPS
 * velocity += gravity * dt;  // Would need scalar multiplication
 * position += velocity * dt; // Would need scalar multiplication
 * ```
 */
template <typename T>
class Vector3d {

private:

    /**
     * @brief Internal storage for the three vector components
     *
     * Uses std::array for type-safe, efficient storage of the x, y, and z
     * components. The array provides:
     * - Bounds checking in debug builds (when using at())
     * - Contiguous memory layout for cache efficiency
     * - Iterator support for range-based operations
     * - No dynamic allocation overhead
     *
     * Component layout: [0] = x, [1] = y, [2] = z
     *
     * @see operator[] for direct component access
     * @see x(), y(), z() for named component access
     */
    std::array<T, 3> data;

public:

    /**
     * @brief Default constructor creating a zero vector
     *
     * Initializes all components to their default values. For numeric types,
     * this typically results in a zero vector (0, 0, 0). The actual initial
     * values depend on the default constructor behavior of type T.
     *
     * @note Uses default initialization, so component values are undefined
     *       for fundamental types unless explicitly initialized by the compiler.
     *       For guaranteed zero initialization, use Vector3d(0, 0, 0).
     *
     * @example
     * ```cpp
     * Vector3d<double> defaultVec; // Components may be uninitialized
     * Vector3d<int> zeroVec(0, 0, 0); // Guaranteed zero vector
     * ```
     */
    Vector3d() {};

    /**
     * @brief Constructor with explicit component values
     *
     * Creates a vector with the specified x, y, and z components. This is the
     * primary way to create vectors with known values and provides explicit
     * control over all three components.
     *
     * @param a The x-component (first element) of the vector
     * @param b The y-component (second element) of the vector
     * @param c The z-component (third element) of the vector
     *
     * @note The parameter names (a, b, c) are generic to avoid confusion with
     *       coordinate system conventions, but correspond to (x, y, z) respectively.
     *
     * @example
     * ```cpp
     * Vector3d<double> position(1.0, 2.0, 3.0);     // Point in space
     * Vector3d<float> direction(0.0f, 1.0f, 0.0f);  // Unit vector pointing up
     * Vector3d<int> gridCell(10, 15, 2);            // Discrete grid coordinates
     * ```
     */
    Vector3d(T a, T b, T c) {
        data[0] = a;
        data[1] = b;
        data[2] = c;
    };

    /**
     * @brief Gets the x-component (first element) of the vector
     *
     * Provides read-only access to the first component of the vector,
     * conventionally representing the x-coordinate in Cartesian space.
     *
     * @return The x-component value by value (copy)
     *
     * @note Returns by value to maintain const-correctness and prevent
     *       accidental modification through the return value.
     *
     * @see operator[] for mutable access to components
     * @see y(), z() for other component accessors
     */
    T x() const { return data[0]; };

    /**
     * @brief Gets the y-component (second element) of the vector
     *
     * Provides read-only access to the second component of the vector,
     * conventionally representing the y-coordinate in Cartesian space.
     *
     * @return The y-component value by value (copy)
     *
     * @note Returns by value to maintain const-correctness and prevent
     *       accidental modification through the return value.
     *
     * @see operator[] for mutable access to components
     * @see x(), z() for other component accessors
     */
    T y() const { return data[1]; };

    /**
     * @brief Gets the z-component (third element) of the vector
     *
     * Provides read-only access to the third component of the vector,
     * conventionally representing the z-coordinate in Cartesian space.
     *
     * @return The z-component value by value (copy)
     *
     * @note Returns by value to maintain const-correctness and prevent
     *       accidental modification through the return value.
     *
     * @see operator[] for mutable access to components
     * @see x(), y() for other component accessors
     */
    T z() const { return data[2]; };

    /**
     * @brief Mutable subscript operator for component access
     *
     * Provides direct access to vector components by index, allowing both
     * reading and modification of individual elements. Index 0 corresponds
     * to x, index 1 to y, and index 2 to z.
     *
     * @param i Index of the component to access (0=x, 1=y, 2=z)
     * @return Reference to the component, allowing modification
     *
     * @warning No bounds checking is performed. Accessing indices outside
     *          [0, 2] results in undefined behavior. Use at() method of the
     *          underlying array for bounds-checked access in debug scenarios.
     *
     * @note This overload is selected for non-const Vector3d objects,
     *       enabling modification of components.
     *
     * @example
     * ```cpp
     * Vector3d<double> vec(1.0, 2.0, 3.0);
     * vec[0] = 5.0;  // Modify x component
     * vec[1] += 1.0; // Increment y component
     * double z = vec[2]; // Read z component
     * ```
     */
    T& operator[] (std::size_t const& i) {
        return data[i];
    }

    /**
     * @brief Const subscript operator for read-only component access
     *
     * Provides read-only access to vector components by index with bounds
     * checking via std::array::at(). This version is used for const Vector3d
     * objects and provides safer access with exception-based error handling.
     *
     * @param i Index of the component to access (0=x, 1=y, 2=z)
     * @return Copy of the component value
     *
     * @throws std::out_of_range if index i is outside the valid range [0, 2]
     *
     * @note This overload is selected for const Vector3d objects and uses
     *       at() for bounds checking, providing exception safety at the
     *       cost of slightly reduced performance.
     *
     * @note Returns by value rather than const reference to maintain
     *       consistency with the x(), y(), z() accessor methods.
     *
     * @example
     * ```cpp
     * const Vector3d<double> vec(1.0, 2.0, 3.0);
     * double x = vec[0]; // Safe read access
     * double invalid = vec[5]; // Throws std::out_of_range
     * ```
     */
    T operator[] (std::size_t const& i) const {
        return data.at(i);
    }

    /**
     * @brief In-place vector addition operator
     *
     * Adds another vector to this vector in-place, modifying the current
     * vector's components. Performs component-wise addition: this.x += that.x,
     * this.y += that.y, this.z += that.z.
     *
     * @param that The vector to add to this vector
     * @return Reference to this vector after modification (enables chaining)
     *
     * @note Modifies the current vector object rather than creating a new one,
     *       providing efficient in-place arithmetic for performance-critical code.
     *
     * @note The return reference enables method chaining like: vec += a += b;
     *
     * @example
     * ```cpp
     * Vector3d<double> position(1.0, 2.0, 3.0);
     * Vector3d<double> velocity(0.1, 0.2, 0.0);
     * position += velocity; // position is now (1.1, 2.2, 3.0)
     *
     * // Method chaining
     * Vector3d<double> total(0.0, 0.0, 0.0);
     * total += vec1 += vec2; // Adds vec2 to vec1, then adds result to total
     * ```
     */
    Vector3d& operator+= (Vector3d const& that) {
        data[0] += that.x();
        data[1] += that.y();
        data[2] += that.z();
        return *this;
    }

    /**
     * @brief In-place vector subtraction operator
     *
     * Subtracts another vector from this vector in-place, modifying the current
     * vector's components. Performs component-wise subtraction: this.x -= that.x,
     * this.y -= that.y, this.z -= that.z.
     *
     * @param that The vector to subtract from this vector
     * @return Reference to this vector after modification (enables chaining)
     *
     * @note Modifies the current vector object rather than creating a new one,
     *       providing efficient in-place arithmetic for performance-critical code.
     *
     * @note The return reference enables method chaining like: vec -= a -= b;
     *
     * @example
     * ```cpp
     * Vector3d<double> position(5.0, 3.0, 1.0);
     * Vector3d<double> displacement(1.0, 1.0, 0.5);
     * position -= displacement; // position is now (4.0, 2.0, 0.5)
     *
     * // Physics: update position by removing old velocity and adding new
     * position -= oldVelocity -= acceleration;
     * ```
     */
    Vector3d& operator-= (Vector3d const& that) {
        data[0] -= that.x();
        data[1] -= that.y();
        data[2] -= that.z();
        return *this;
    }

    /**
     * @brief Vector addition operator (non-modifying)
     *
     * Creates a new vector that is the sum of this vector and another vector.
     * Performs component-wise addition without modifying either operand vector.
     * The result vector has components: (this.x + that.x, this.y + that.y, this.z + that.z).
     *
     * @param that The vector to add to this vector
     * @return New Vector3d object containing the sum
     *
     * @note Creates a new vector object, leaving both operands unchanged.
     *       For performance-critical code where in-place modification is
     *       acceptable, consider using operator+= instead.
     *
     * @note This is a binary operator that enables natural mathematical
     *       syntax for vector arithmetic expressions.
     *
     * @example
     * ```cpp
     * Vector3d<double> a(1.0, 2.0, 3.0);
     * Vector3d<double> b(4.0, 5.0, 6.0);
     * Vector3d<double> c = a + b; // c is (5.0, 7.0, 9.0)
     * // a and b remain unchanged
     *
     * // Complex expressions
     * Vector3d<double> result = position + velocity + acceleration;
     * ```
     */
    Vector3d operator+ (Vector3d const& that) {
        return Vector3d(x() + that.x(), y() + that.y(), z() + that.z());
    }

    /**
     * @brief Vector subtraction operator (non-modifying)
     *
     * Creates a new vector that is the difference of this vector and another vector.
     * Performs component-wise subtraction without modifying either operand vector.
     * The result vector has components: (this.x - that.x, this.y - that.y, this.z - that.z).
     *
     * @param that The vector to subtract from this vector
     * @return New Vector3d object containing the difference
     *
     * @note Creates a new vector object, leaving both operands unchanged.
     *       For performance-critical code where in-place modification is
     *       acceptable, consider using operator-= instead.
     *
     * @note This operation represents vector subtraction, which geometrically
     *       gives the vector from 'that' to 'this' when vectors represent positions.
     *
     * @example
     * ```cpp
     * Vector3d<double> endPos(10.0, 8.0, 5.0);
     * Vector3d<double> startPos(2.0, 3.0, 1.0);
     * Vector3d<double> displacement = endPos - startPos; // (8.0, 5.0, 4.0)
     * // endPos and startPos remain unchanged
     *
     * // Physics: calculate relative position
     * Vector3d<double> relativePos = playerPos - enemyPos;
     * ```
     */
    Vector3d operator- (Vector3d const& that) {
        return Vector3d(x() - that.x(), y() - that.y(), z() - that.z());
    }

};