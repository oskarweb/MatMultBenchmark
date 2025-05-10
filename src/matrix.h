#pragma once

#include <array>
#include <random>
#include <vector>

#include "utils.h"


template<typename MatrixA, typename MatrixB>
concept CompatibleMatrices = (MatrixA::Columns == MatrixB::Rows);

template<typename DataType = uint64_t, uint32_t _Rows = 2, uint32_t _Cols = 2>
class Matrix { 
public:
    constexpr const static uint32_t Rows = _Rows;
    constexpr const static uint32_t Columns = _Cols;
    
    Matrix() {
        std::ranges::for_each(m_data, [](auto &row){
            row.fill(static_cast<DataType>(0));
        });
    }

    Matrix(DataType value) {
        std::ranges::for_each(m_data, [](auto &row){
            row.fill(value);
        });
    }

    template<typename OtherMatrix>
    requires CompatibleMatrices<Matrix<DataType, Rows, Columns>, OtherMatrix>
    Matrix<DataType, Rows, OtherMatrix::Columns> mult(const OtherMatrix &other) const {
        Matrix<DataType, Rows, OtherMatrix::Columns> result;
        
        for (uint32_t i = 0; i < Rows; ++i) {
            for (uint32_t j = 0; j < OtherMatrix::Columns; ++j) {
                for (uint32_t k = 0; k < Columns; ++k) {
                    result.m_data[i][j] += this->m_data[i][k] * other.m_data[k][j];
                }
            }
        }

        return result;
    }

    void randomFill(DataType min, DataType max) {
        static std::mt19937 gen(1);
        static std::uniform_int_distribution<DataType> distrib(1, 100);

        std::ranges::for_each(m_data, [](auto &row){
            std::generate(row.begin(), row.end(), [] { return distrib(gen); });
        });
    }

    void randomFill() {
        randomFill(0, 100);
    }

    std::array<std::array<DataType, Columns>, Rows> &data() {
        return m_data;
    }

    const std::array<std::array<DataType, Columns>, Rows> &get() const {
        return m_data;
    }

private:
    std::array<std::array<DataType, Columns>, Rows> m_data;
};

template<ElementIterable T, typename D>
std::vector<std::vector<D>> matMult(const T &matA, const T &matB) 
{
    if (matA[0].size() != matB.size()) 
    {
        std::cerr << "Mat A columns != Mat B rows\n";
        return {{}};
    }
    auto m = matA.size();
    auto p = matB[0].size();
    auto n = matB.size();
    T matC(m, std::vector<T>(p, 0));
    for(size_t i = 0; i < m; ++i) 
    {
        for (size_t j = 0; j < p; ++j) 
        {
            for (size_t k = 0; k < n; ++k)
            {
                matC[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }
    return matC;
}

template<typename D, size_t Order>
std::array<std::array<D, Order>, Order> matMult(
    const std::array<std::array<D, Order>, Order> &matA, 
    const std::array<std::array<D, Order>, Order> &matB) 
{
    if (matA[0].size() != matB.size()) 
    {
        return {{}};
    }
    auto m = matA.size();
    auto p = matB[0].size();
    auto n = matB.size();
    std::array<std::array<D, Order>, Order> matC;
    for(size_t i = 0; i < m; ++i) 
    {
        for (size_t j = 0; j < p; ++j) 
        {
            for (size_t k = 0; k < n; ++k)
            {
                matC[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }
    return matC;
}

// template<typename D, size_t Order>
// std::array<std::array<D, Order>, Order> matMult(
//     const std::array<std::array<D, Order>, Order> &matA, 
//     const std::array<std::array<D, Order>, Order> &matB) 
// {
//     if (matA[0].size() != matB.size()) 
//     {
//         return {{}};
//     }
//     auto m = matA.size();
//     auto p = matB[0].size();
//     auto n = matB.size();
//     std::array<std::array<D, Order>, Order> matC;
//     for(size_t i = 0; i < m; ++i) 
//     {
//         for (size_t j = 0; j < p; ++j) 
//         {
//             for (size_t k = 0; k < n; ++k)
//             {
//                 matC[i][j] += matA[i][k] * matB[k][j];
//             }
//         }
//     }
//     return matC;
// }

template<typename T, uint32_t Order = 2>
std::array<std::array<T, Order>, Order> genRandomMatrix() {
    static std::mt19937 gen(1);
    static std::uniform_int_distribution<> distrib(1, 100);

    std::array<std::array<T, Order>, Order> mat; 
    for (auto &row : mat) 
    {
        std::generate(row.begin(), row.end(), [] { return static_cast<T>(distrib(gen)); });
    }
    return mat;
}

template<ElementIterable T>
void printMatrix(const T& mat) {
    for (auto row_it = mat.begin(); row_it != mat.end(); ++row_it)
    {
        for (auto element_it = (*row_it).begin(); element_it != (*row_it).end(); ++element_it)
        {
            std::cout << *element_it;
            if (element_it != (*row_it).end() - 1)
                std::cout << " ";
        }
        std::cout << '\n';
    }
}