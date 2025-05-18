#pragma once

#include "utils.h"
#include "simd_traits.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

template<typename MatrixA, typename MatrixB>
concept CompatibleMatrices = (MatrixA::Columns == MatrixB::Rows);

template<typename MatrixA, typename MatrixB>
concept EqualDims = (MatrixA::Rows == MatrixB::Rows) && (MatrixA::Columns == MatrixB::Columns);

enum class MultType 
{
    Naive = 0,
    Simd,
    MultithreadRow,
    MultithreadElement,
    MultithreadSimd,
};

template<typename DataType = uint32_t, uint32_t _Rows = 2, uint32_t _Cols = 2>
class Matrix 
{ 
public:
    template <MultType, typename MatrixA, typename MatrixB>
    struct MatrixMultImpl;
    
    constexpr const static uint32_t Rows = _Rows;
    constexpr const static uint32_t Columns = _Cols;

    Matrix(DataType value) { std::ranges::fill(m_data, value); }

    Matrix() : Matrix(static_cast<DataType>(0)) {}

    template<MultType multType, typename OtherMatrix>
    requires CompatibleMatrices<Matrix<DataType, Rows, Columns>, OtherMatrix>
    Matrix<DataType, Rows, OtherMatrix::Columns> mult(const OtherMatrix& other) const 
    {
        return MatrixMultImpl<multType, Matrix, OtherMatrix>::multiply(*this, other);
    }

    void randomFill(DataType min, DataType max) 
    {
        static std::mt19937 gen(1);

        if constexpr (std::is_integral_v<DataType>)
        {
            std::uniform_int_distribution<DataType> distrib(min, max);
            std::generate(m_data.begin(), m_data.end(), [&]() { return distrib(gen); });
        } else if constexpr (std::is_floating_point_v<DataType>) 
        {
            std::uniform_real_distribution<DataType> distrib(min, max);
            std::generate(m_data.begin(), m_data.end(), [&]() { return distrib(gen); });
        } else 
        {
            static_assert(std::is_arithmetic_v<DataType>, "Unsupported data type for randomFill.");
        }
    }

    void randomFill() { randomFill(static_cast<DataType>(0), static_cast<DataType>(100)); }

    std::array<DataType, Rows * Columns> &data() { return m_data; }
    const std::array<DataType, Rows * Columns> &data() const { return m_data; }

    DataType &operator()(size_t i, size_t j) { return m_data[i * Columns + j]; }
    const DataType &operator()(size_t i, size_t j) const { return m_data[i * Columns + j]; }

    DataType &operator[](size_t i) { return m_data[i]; }
    const DataType &operator[](size_t i) const { return m_data[i]; }
    
    template<typename OtherMatrix>
    requires EqualDims<Matrix<DataType, Rows, Columns>, OtherMatrix>
    bool operator==(const OtherMatrix &other) const { return std::ranges::equal(this->m_data, other.data()); }
private:
    std::array<DataType, Rows * Columns> m_data;
};

#define A(i,j) a[(i) * lda + (j)]
#define B(i,j) b[(i) * ldb + (j)]
#define C(i,j) c[(i) * ldc + (j)]

template <typename T, unsigned RA, unsigned RB>
void matmul_dot_inner(int k, const T* a, int lda, const T* b, int ldb, T* c, int ldc) 
{
    using Simd = SimdTraits<T>;
    using Vec = typename Simd::vec;
    constexpr int W = Simd::width;

    Vec csum[RA][RB] = {};

    for (int p = 0; p < k; ++p) 
    {
        for (int bi = 0; bi < RB; ++bi) 
        {
            Vec bb = Simd::load(&B(p, bi * W));
            for (int ai = 0; ai < RA; ++ai) 
            {
                Vec aa = Simd::broadcast(A(ai, p));
                csum[ai][bi] = Simd::add(csum[ai][bi], Simd::mul(aa, bb));
            }
        }
    }

    for (int ai = 0; ai < RA; ++ai) 
    {
        for (int bi = 0; bi < RB; ++bi) 
        {
            Simd::store(&C(ai, bi * W), csum[ai][bi]);
        }
    }
}

template <typename DataType, uint32_t Rows, uint32_t Columns>
template <typename MatrixA, typename MatrixB>
struct Matrix<DataType, Rows, Columns>::MatrixMultImpl<MultType::Naive, MatrixA, MatrixB> 
{
    static auto multiply(const MatrixA& a, const MatrixB& b) 
    {
        using ResultMatrix = Matrix<DataType, MatrixA::Rows, MatrixB::Columns>;
        ResultMatrix result;

        for (uint32_t i = 0; i < MatrixA::Rows; ++i) 
        {
            for (uint32_t j = 0; j < MatrixB::Columns; ++j) 
            {
                for (uint32_t k = 0; k < MatrixA::Columns; ++k) 
                {
                    result(i, j) += a(i, k) * b(k, j);
                }
            }
        }

        return result;
    }
};

template <typename DataType, uint32_t Rows, uint32_t Columns>
template <typename MatrixA, typename MatrixB>
struct Matrix<DataType, Rows, Columns>::MatrixMultImpl<MultType::Simd, MatrixA, MatrixB> 
{
    static auto multiply(const MatrixA& a, const MatrixB& b) 
    {
        using ResultMatrix = Matrix<DataType, MatrixA::Rows, MatrixB::Columns>;
        ResultMatrix result;
        constexpr uint32_t regsA = 3;
        constexpr uint32_t regsB = 4;
        constexpr uint32_t blockCols = regsB * SimdTraits<DataType>::width;
        constexpr uint32_t M = MatrixA::Rows; // = 32 - 3 = 29    (M - RA) % RA == 0 
        constexpr uint32_t N = MatrixB::Columns; // = 32 - 32 = 0
        constexpr uint32_t K = MatrixA::Columns;
        constexpr uint32_t lda = MatrixA::Columns;
        constexpr uint32_t ldb = MatrixB::Columns;
        constexpr uint32_t ldc = MatrixB::Columns;

        for (int i = 0; i <= M - regsA; i += regsA) 
        {
            for (int j = 0; j <= N - blockCols; j += blockCols) 
            {
                matmul_dot_inner<DataType, regsA, regsB>(
                    K,
                    &a[i * lda], lda,
                    &b[j], ldb,
                    &result[i * ldc + j], ldc
                );
            }
        }

        for (uint32_t i = M - M % regsA; i < M; ++i)
            for (uint32_t j = 0; j < N; ++j)
                for (uint32_t k = 0; k < K; ++k)
                    result[i * ldc + j] += a[i * lda + k] * b[k * ldb + j];


        for (uint32_t i = 0; i <= M - regsA; i += regsA)
            for (uint32_t j = N - N % blockCols; j < N; ++j)
                for (uint32_t k = 0; k < K; ++k)
                    for (uint32_t ii = 0; ii < regsA; ++ii)
                        result[(i + ii) * ldc + j] += a[(i + ii) * lda + k] * b[k * ldb + j];
        
        return result;
    }
};

template <typename DataType, uint32_t Rows, uint32_t Columns>
template <typename MatrixA, typename MatrixB>
struct Matrix<DataType, Rows, Columns>::MatrixMultImpl<MultType::MultithreadRow, MatrixA, MatrixB> 
{
    static auto multiply(const MatrixA& a, const MatrixB& b) 
    {
        using ResultMatrix = Matrix<DataType, MatrixA::Rows, MatrixB::Columns>;
        ResultMatrix result;
        constexpr uint32_t M = MatrixA::Rows;
        constexpr uint32_t N = MatrixB::Columns;
        constexpr uint32_t K = MatrixA::Columns;

        std::array<std::thread, M> threads;

        for(uint32_t i = 0; i < M; ++i) 
        {
            threads[i] = std::thread([&a, &b, &result, i] ()
            {
                for (uint32_t j = 0; j < N; ++j) 
                {
                    for (uint32_t k = 0; k < K; ++k)
                    {
                        result[i * N + j] += a[i * K + k] * b[k * N + j];
                    }
                }
            });
        }

        for (auto it = threads.begin(); it < threads.end(); ++it) 
        {
            (*it).join();
        }

        return result;
    }
};

template <typename DataType, uint32_t Rows, uint32_t Columns>
template <typename MatrixA, typename MatrixB>
struct Matrix<DataType, Rows, Columns>::MatrixMultImpl<MultType::MultithreadElement, MatrixA, MatrixB> 
{
    static auto multiply(const MatrixA& a, const MatrixB& b) 
    {
        using ResultMatrix = Matrix<DataType, MatrixA::Rows, MatrixB::Columns>;
        ResultMatrix result;
        constexpr uint32_t M = MatrixA::Rows;
        constexpr uint32_t N = MatrixB::Columns;
        constexpr uint32_t K = MatrixA::Columns;

        std::array<std::thread, M * N> threads;

        for(uint32_t i = 0; i < M; ++i) 
        {
            for (uint32_t j = 0; j < N; ++j) 
            {
                threads[i * K + j] = std::thread([&a, &b, &result, i, j] () 
                {
                    for (uint32_t k = 0; k < K; ++k)
                    {
                        result[i * N + j] += a[i * K + k] * b[k * N + j];
                    }
                });
            }
        }

        for (auto it = threads.begin(); it < threads.end(); ++it) 
        {
            (*it).join();
        }

        return result;
    }
};

template <typename DataType, uint32_t Rows, uint32_t Columns>
template <typename MatrixA, typename MatrixB>
struct Matrix<DataType, Rows, Columns>::MatrixMultImpl<MultType::MultithreadSimd, MatrixA, MatrixB> 
{
    static auto multiply(const MatrixA& a, const MatrixB& b) 
    {
        using ResultMatrix = Matrix<DataType, MatrixA::Rows, MatrixB::Columns>;
        ResultMatrix result;
        constexpr uint32_t regsA = 3;
        constexpr uint32_t regsB = 4;
        constexpr uint32_t blockCols = regsB * SimdTraits<DataType>::width;
        constexpr uint32_t M = MatrixA::Rows;
        constexpr uint32_t N = MatrixB::Columns;
        constexpr uint32_t K = MatrixA::Columns;
        constexpr uint32_t lda = MatrixA::Columns;
        constexpr uint32_t ldb = MatrixB::Columns;
        constexpr uint32_t ldc = MatrixB::Columns;
        constexpr uint32_t numRowBlocks = M / regsA;
        constexpr uint32_t numColBlocks = N / blockCols;
        
        const uint32_t threadCount = std::max(4u, std::thread::hardware_concurrency());
        std::vector<std::thread> threads(threadCount);

        std::vector<uint32_t> startRows(threadCount + 1);
        uint32_t blocksPerThreadBase = numRowBlocks / threadCount;
        uint32_t blocksPerThreadRemainder = numRowBlocks % threadCount;
        for (uint32_t tid = 0; tid < startRows.size(); tid++)
        {
            startRows[tid] = tid * blocksPerThreadBase + std::min(tid, blocksPerThreadRemainder);
        }
        startRows[threadCount] = numRowBlocks;

        for (uint32_t tid = 0; tid < threadCount; ++tid) 
        {
            threads[tid] = std::thread([tid, &startRows, &a, &b, &result, lda, ldb, ldc]() 
            {
                uint32_t startRow = startRows[tid];
                uint32_t endRow = startRows[tid + 1];
            
                for (uint32_t blockRow = startRow; blockRow < endRow; ++blockRow) 
                {
                    uint32_t i = blockRow * regsA;
                    for (uint32_t blockCol = 0; blockCol < numColBlocks; ++blockCol) 
                    {
                        uint32_t j = blockCol * blockCols;
                        matmul_dot_inner<DataType, regsA, regsB>(
                            K,
                            &a[i * lda], lda,
                            &b[j], ldb,
                            &result[i * ldc + j], ldc
                        );
                    }
                }

                if (tid == 0) 
                {
                    for (uint32_t i = M - M % regsA; i < M; ++i)
                        for (uint32_t j = 0; j < N; ++j)
                            for (uint32_t k = 0; k < K; ++k)
                                result[i * ldc + j] += a[i * lda + k] * b[k * ldb + j];
        
        
                    for (uint32_t i = 0; i <= M - regsA; i += regsA)
                        for (uint32_t j = N - N % blockCols; j < N; ++j)
                            for (uint32_t k = 0; k < K; ++k)
                                for (uint32_t ii = 0; ii < regsA; ++ii)
                                    result[(i + ii) * ldc + j] += a[(i + ii) * lda + k] * b[k * ldb + j];
                }
            });
        }

        for (auto it = threads.begin(); it < threads.end(); ++it) 
        {
            (*it).join();
        }
        
        return result;
    }
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

template<typename T, uint32_t Order = 2>
std::array<std::array<T, Order>, Order> genRandomMatrix() 
{
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
void printMatrix(const T& mat)
{
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