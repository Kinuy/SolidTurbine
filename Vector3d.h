#pragma once

#include <array>

//Template implementation of 3d vector for storing 3D coordinates
template <typename T>
class Vector3d {

private:

    std::array<T, 3> data;

public:

    Vector3d() {};

    Vector3d(T a, T b, T c) {
        data[0] = a;
        data[1] = b;
        data[2] = c;
    };
    T x() const { return data[0]; };
    T y() const { return data[1]; };
    T z() const { return data[2]; };
    T& operator[] (std::size_t const& i) {
        return data[i];
    }
    T operator[] (std::size_t const& i) const {
        return data.at(i);
    }
    Vector3d& operator+= (Vector3d const& that) {
        data[0] += that.x();
        data[1] += that.y();
        data[2] += that.z();
        return *this;
    }
    Vector3d& operator-= (Vector3d const& that) {
        data[0] -= that.x();
        data[1] -= that.y();
        data[2] -= that.z();
        return *this;
    }
    Vector3d operator+ (Vector3d const& that) {
        return Vector3d(x() + that.x(), y() + that.y(), z() + that.z());
    }
    Vector3d operator- (Vector3d const& that) {
        return Vector3d(x() - that.x(), y() - that.y(), z() - that.z());
    }

};