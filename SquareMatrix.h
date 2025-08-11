#pragma once

#include <vector>

// Template implementation of square matrix for any type T
template <typename T>
class SquareMatrix {

private:

    unsigned int dim_;
    std::vector<T> data_;

public:

    // no dim specifier gives a 3x3 matrix:
    SquareMatrix()
        : dim_(3) {
        data_.resize(dim_ * dim_);
        for (std::size_t i = 0; i < data_.size(); ++i) {
            data_[i] = 0;
        }
    };

    // create square matrix of zeros at any dimension:
    SquareMatrix(unsigned int dim)
        : dim_(dim) {
        data_.resize(dim * dim);
        for (std::size_t i = 0; i < data_.size(); ++i) {
            data_[i] = 0;
        }
    };

    // can create a 3x3 with the values:
    SquareMatrix(T r1c1,
        T r1c2,
        T r1c3,
        T r2c1,
        T r2c2,
        T r2c3,
        T r3c1,
        T r3c2,
        T r3c3)
        : dim_(3) {
        data_.resize(dim_ * dim_);
        data_[0] = r1c1;
        data_[1] = r1c2;
        data_[2] = r1c3;
        data_[3] = r2c1;
        data_[4] = r2c2;
        data_[5] = r2c3;
        data_[6] = r3c1;
        data_[7] = r3c2;
        data_[8] = r3c3;
    }

    void makeIdentity() {
        for (std::size_t i = 0; i < dim_; ++i) {
            for (std::size_t j = 0; j < dim_; ++j) {
                data_[i * dim_ + j] = 0;
                if (i == j) {
                    data_[i * dim_ + j] = 1;
                }
            }
        }
    };
    T& operator[] (std::size_t const& i) {
        return data_[i];
    }
    T operator[] (std::size_t const& i) const {
        return data_[i];
    }

    void transpose() {
        std::vector<T> temp_data;
        for (std::size_t i = 0; i < dim_; ++i) {
            for (std::size_t j = 0; j < dim_; ++j) {
                temp_data.push_back(data_[j * dim_ + i]);
            }
        }
        data_ = temp_data;
    }

    unsigned int dim() const { return dim_; };

    std::vector<T> data() const { return data_; };

};
