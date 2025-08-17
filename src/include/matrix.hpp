#pragma once

#include "constants.hpp"
#include "ocl_utils.hpp"
#include "utils.hpp"
#include "simd_traits.hpp"

#include <CL/cl.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

template<typename Mat>
concept ValidDims = (Mat::Columns > 1) && (Mat::Rows > 1);

template<typename MatrixA, typename MatrixB>
concept CompatibleMatrices = (MatrixA::Columns == MatrixB::Rows);

template<typename MatrixA, typename MatrixB>
concept EqualDims = (MatrixA::Rows == MatrixB::Rows) && (MatrixA::Columns == MatrixB::Columns);

enum class MatMultType 
{
    Naive = 0,
    Simd,
    MultithreadRow,
    MultithreadElement,
    MultithreadSimd,
    NaiveOcl,
    Last
};

enum class MatMultDataType 
{ 
    Int32 = 0,
    Uint32,
    Float,
    Double,
    Last 
};

template<typename T, size_t Size>
struct MatrixStorage {
    static constexpr bool stackAllocation = false; // (sizeof(T) * Size <= constants::getStackSize() / 100);

    using type = std::conditional_t<
        stackAllocation,
        std::array<T, Size>,
        std::vector<T>
    >;
};

template <MatMultType, typename MatrixA, typename MatrixB>
struct MatrixMultImpl;

template<typename DataType = int, 
    int _Rows = constants::DEFAULT_MATRIX_ORDER, 
    int _Cols = constants::DEFAULT_MATRIX_ORDER>
class Matrix 
{ 
public:
    constexpr const static int Rows = _Rows;
    constexpr const static int Columns = _Cols;
    constexpr const static int Size = Rows * Columns;
    
    using DataT = typename DataType;
    using Storage = typename MatrixStorage<DataType, Size>::type;

    template<typename T = void>
    requires ValidDims<Matrix>
    Matrix(DataType value)
    {
        if constexpr (std::is_same_v<Storage, std::vector<DataType>>) 
        {
            m_data.resize(Size, value);
        } else 
        {
            std::ranges::fill(m_data, value);
        }
    }

    template<typename T = void>
    requires ValidDims<Matrix>
    Matrix() : Matrix(static_cast<DataType>(0)) {}

    template<MatMultType MultType, typename OtherMatrix>
    requires CompatibleMatrices<Matrix<DataType, Rows, Columns>, OtherMatrix>
    Matrix<DataType, Rows, OtherMatrix::Columns> mult(const OtherMatrix& other) const 
    {
        return MatrixMultImpl<MultType, Matrix, OtherMatrix>::multiply(*this, other);
    }

    void randomFill(DataType min, DataType max, unsigned int seed) 
    {
        static std::mt19937 gen(seed);

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

    void randomFill() { randomFill(static_cast<DataType>(0), static_cast<DataType>(100), 1u); }
    void randomFill(unsigned int seed) { randomFill(static_cast<DataType>(0), static_cast<DataType>(100), seed); }

    Storage &data() { return m_data; }
    const Storage &data() const { return m_data; }

    DataType &operator()(size_t i, size_t j) { return m_data[i * Columns + j]; }
    const DataType &operator()(size_t i, size_t j) const { return m_data[i * Columns + j]; }

    DataType &operator[](size_t i) { return m_data[i]; }
    const DataType &operator[](size_t i) const { return m_data[i]; }
    
    template<typename OtherMatrix>
    requires EqualDims<Matrix<DataType, Rows, Columns>, OtherMatrix>
    bool operator==(const OtherMatrix &other) const { return std::ranges::equal(this->m_data, other.data()); }

#define A(i,j) a[(i) * lda + (j)]
#define B(i,j) b[(i) * ldb + (j)]
#define C(i,j) c[(i) * ldc + (j)]

    template <typename T, unsigned RA, unsigned RB>
    static void matmul_dot_inner(int k, const T* a, int lda, const T* b, int ldb, T* c, int ldc) 
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

#undef A
#undef B
#undef C

private:
    Storage m_data;
};

template <typename MatrixA, typename MatrixB>
struct MatrixMultImpl<MatMultType::Naive, MatrixA, MatrixB> 
{
    static auto multiply(const MatrixA& a, const MatrixB& b) 
    {
        using ResultMatrix = Matrix<MatrixA::DataT, MatrixA::Rows, MatrixB::Columns>;
        ResultMatrix result;

        for (int i = 0; i < MatrixA::Rows; ++i) 
        {
            for (int j = 0; j < MatrixB::Columns; ++j) 
            {
                for (int k = 0; k < MatrixA::Columns; ++k) 
                {
                    result(i, j) += a(i, k) * b(k, j);
                }
            }
        }

        return result;
    }
};


template <typename MatrixA, typename MatrixB>
struct MatrixMultImpl<MatMultType::Simd, MatrixA, MatrixB> 
{
    static auto multiply(const MatrixA& a, const MatrixB& b) 
    {
        using ResultMatrix = Matrix<MatrixA::DataT, MatrixA::Rows, MatrixB::Columns>;
        ResultMatrix result;
        constexpr int regsA = 3;
        constexpr int regsB = 4;
        constexpr int blockCols = regsB * SimdTraits<MatrixA::DataT>::width;
        constexpr int M = MatrixA::Rows;
        constexpr int N = MatrixB::Columns;
        constexpr int K = MatrixA::Columns;
        constexpr int lda = MatrixA::Columns;
        constexpr int ldb = MatrixB::Columns;
        constexpr int ldc = MatrixB::Columns;

        for (int i = 0; i <= M - regsA; i += regsA) 
        {
            for (int j = 0; j <= N - blockCols; j += blockCols) 
            {
                Matrix<>::matmul_dot_inner<MatrixA::DataT, regsA, regsB>(
                    K,
                    &a[i * lda], lda,
                    &b[j], ldb,
                    &result[i * ldc + j], ldc
                );
            }
        }

        for (int i = M - M % regsA; i < M; ++i)
            for (int j = 0; j < N; ++j)
                for (int k = 0; k < K; ++k)
                    result[i * ldc + j] += a[i * lda + k] * b[k * ldb + j];

        for (int i = 0; i <= M - regsA; i += regsA)
            for (int j = N - N % blockCols; j < N; ++j)
                for (int k = 0; k < K; ++k)
                    for (int ii = 0; ii < regsA; ++ii)
                        result[(i + ii) * ldc + j] += a[(i + ii) * lda + k] * b[k * ldb + j];
        
        return result;
    }
};


template <typename MatrixA, typename MatrixB>
struct MatrixMultImpl<MatMultType::MultithreadRow, MatrixA, MatrixB> 
{
    static auto multiply(const MatrixA& a, const MatrixB& b) 
    {
        using ResultMatrix = Matrix<MatrixA::DataT, MatrixA::Rows, MatrixB::Columns>;
        ResultMatrix result;
        constexpr int M = MatrixA::Rows;
        constexpr int N = MatrixB::Columns;
        constexpr int K = MatrixA::Columns;

        std::vector<std::thread> threads(M);

        for(int i = 0; i < M; ++i) 
        {
            threads[i] = std::thread([&a, &b, &result, i] ()
            {
                for (int j = 0; j < N; ++j) 
                {
                    for (int k = 0; k < K; ++k)
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


template <typename MatrixA, typename MatrixB>
struct MatrixMultImpl<MatMultType::MultithreadElement, MatrixA, MatrixB> 
{
    static auto multiply(const MatrixA& a, const MatrixB& b) 
    {
        using ResultMatrix = Matrix<MatrixA::DataT, MatrixA::Rows, MatrixB::Columns>;
        ResultMatrix result;
        constexpr int M = MatrixA::Rows;
        constexpr int N = MatrixB::Columns;
        constexpr int K = MatrixA::Columns;

        std::vector<std::thread> threads(M*N);

        for(int i = 0; i < M; ++i) 
        {
            for (int j = 0; j < N; ++j) 
            {
                threads[i * N + j] = std::thread([&a, &b, &result, i, j] () 
                {
                    for (int k = 0; k < K; ++k)
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


template <typename MatrixA, typename MatrixB>
struct MatrixMultImpl<MatMultType::MultithreadSimd, MatrixA, MatrixB> 
{
    static auto multiply(const MatrixA& a, const MatrixB& b) 
    {
        using ResultMatrix = Matrix<MatrixA::DataT, MatrixA::Rows, MatrixB::Columns>;
        ResultMatrix result;
        constexpr int regsA = 3;
        constexpr int regsB = 4;
        constexpr int blockCols = regsB * SimdTraits<MatrixA::DataT>::width;
        constexpr int M = MatrixA::Rows;
        constexpr int N = MatrixB::Columns;
        constexpr int K = MatrixA::Columns;
        constexpr int lda = MatrixA::Columns;
        constexpr int ldb = MatrixB::Columns;
        constexpr int ldc = MatrixB::Columns;
        constexpr int numRowBlocks = M / regsA;
        constexpr int numColBlocks = N / blockCols;
        
        const int threadCount = std::max(4u, std::thread::hardware_concurrency());
        std::vector<std::thread> threads(threadCount);

        std::vector<int> startRows(threadCount + 1);
        int blocksPerThreadBase = numRowBlocks / threadCount;
        int blocksPerThreadRemainder = numRowBlocks % threadCount;
        for (int tid = 0; tid < startRows.size(); tid++)
        {
            startRows[tid] = tid * blocksPerThreadBase + std::min(tid, blocksPerThreadRemainder);
        }
        startRows[threadCount] = numRowBlocks;

        for (int tid = 0; tid < threadCount; ++tid) 
        {
            threads[tid] = std::thread([tid, &startRows, &a, &b, &result, lda, ldb, ldc]() 
            {
                int startRow = startRows[tid];
                int endRow = startRows[tid + 1];
            
                for (int blockRow = startRow; blockRow < endRow; ++blockRow) 
                {
                    int i = blockRow * regsA;
                    for (int blockCol = 0; blockCol < numColBlocks; ++blockCol) 
                    {
                        int j = blockCol * blockCols;
                        Matrix<>::matmul_dot_inner<MatrixA::DataT, regsA, regsB>(
                            K,
                            &a[i * lda], lda,
                            &b[j], ldb,
                            &result[i * ldc + j], ldc
                        );
                    }
                }

                if (tid == 0) 
                {
                    for (int i = M - M % regsA; i < M; ++i)
                        for (int j = 0; j < N; ++j)
                            for (int k = 0; k < K; ++k)
                                result[i * ldc + j] += a[i * lda + k] * b[k * ldb + j];
        
        
                    for (int i = 0; i <= M - regsA; i += regsA)
                        for (int j = N - N % blockCols; j < N; ++j)
                            for (int k = 0; k < K; ++k)
                                for (int ii = 0; ii < regsA; ++ii)
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


template <typename MatrixA, typename MatrixB>
struct MatrixMultImpl<MatMultType::NaiveOcl, MatrixA, MatrixB> 
{
    static auto multiply(const MatrixA& a, const MatrixB& b) 
    {
        using ResultMatrix = Matrix<MatrixA::DataT, MatrixA::Rows, MatrixB::Columns>;
        ResultMatrix result;
        cl_int err;
        
        oclUtil::ProgramWithQueue program{"mat_mult.cl"};
        program.initialize();
        program.loadFromBinary();
        program.createQueueAndKernel("naive_mat_mult");

        oclUtil::memWrapper bufA = clCreateBuffer(program.getContext(),
                                                  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                  sizeof(MatrixA::DataT) * MatrixA::Size,
                                                  const_cast<MatrixA::DataT*>(a.data().data()), 
                                                  &err);
        CHECK_CL_ERROR(err, "clCreateBuffer");

        oclUtil::memWrapper bufB = clCreateBuffer(program.getContext(), 
                                                  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                                  sizeof(MatrixA::DataT) * MatrixB::Size, 
                                                  const_cast<MatrixA::DataT*>(b.data().data()), 
                                                  &err);
        CHECK_CL_ERROR(err, "clCreateBuffer");

        oclUtil::memWrapper bufC = clCreateBuffer(program.getContext(), 
                                                  CL_MEM_WRITE_ONLY, 
                                                  sizeof(MatrixA::DataT) * ResultMatrix::Size, 
                                                  nullptr, 
                                                  &err);
        CHECK_CL_ERROR(err, "clCreateBuffer");
    
        clSetKernelArg(program.getKernel(), 0, sizeof(cl_mem), &bufA);
        clSetKernelArg(program.getKernel(), 1, sizeof(cl_mem), &bufB);
        clSetKernelArg(program.getKernel(), 2, sizeof(cl_mem), &bufC);
        clSetKernelArg(program.getKernel(), 3, sizeof(int), &MatrixA::Rows);
        clSetKernelArg(program.getKernel(), 4, sizeof(int), &MatrixA::Columns);
        clSetKernelArg(program.getKernel(), 5, sizeof(int), &ResultMatrix::Columns);

        const size_t globalWorkSize[2] = {ResultMatrix::Rows, ResultMatrix::Columns};
        err = clEnqueueNDRangeKernel(program.getCmdQueue(), 
                                     program.getKernel(), 
                                     2, 
                                     nullptr, 
                                     globalWorkSize, 
                                     nullptr, 
                                     0, 
                                     nullptr, 
                                     nullptr);
        CHECK_CL_ERROR(err, "clEnqueueNDRangeKernel");
    
        clEnqueueReadBuffer(program.getCmdQueue(), 
                            bufC, 
                            CL_TRUE, 
                            0, 
                            sizeof(MatrixA::DataT) * ResultMatrix::Size, 
                            result.data().data(), 
                            0, 
                            nullptr,
                            nullptr);

        return result;
    }
};

template<util::ElementIterable T, typename D>
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

template<typename T, int Order = 2>
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

template<util::ElementIterable T>
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