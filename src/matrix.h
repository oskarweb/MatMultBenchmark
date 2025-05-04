#pragma once

#include <array>
#include <random>
#include <vector>

#include "utils.h"

template<ElementIterable T, typename D>
std::vector<std::vector<D>> matMult(const T &matA, const T &matB) 
{
    if (matA[0].size() != matB.size()) 
    {
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