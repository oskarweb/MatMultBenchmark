#include "test_utils.hpp"

#include "matrix.hpp"

#include <gtest/gtest.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/operation.hpp>

#include <ranges>

#define MATRIX_DIMS_FOR_TYPE(T)                     \
    MatrixParams<T, 2, 2>,                          \
    MatrixParams<T, 4, 4>,                          \
    MatrixParams<T, 32, 32>,                        \
    MatrixParams<T, 128, 128>

template<typename T>
void randomfillBoostMatrix(boost::numeric::ublas::matrix<T>& mat, T min, T max)
{
    if constexpr (std::is_integral_v<T>)
    {
        std::uniform_int_distribution<T> distrib(min, max);
        std::generate(mat.data().begin(), mat.data().end(), [&]() { return distrib(GlobalRNG::get()); });
    } else if constexpr (std::is_floating_point_v<T>) 
    {
        std::uniform_real_distribution<T> distrib(min, max);
        std::generate(mat.data().begin(), mat.data().end(), [&]() { return distrib(GlobalRNG::get()); });
    } else 
    {
        static_assert(std::is_arithmetic_v<T>, "Unsupported data type.");
    }
}

template <typename T, int _Rows, int _Cols>
struct MatrixParams {
    using DataType = T;
    static constexpr int Rows = _Rows;
    static constexpr int Cols = _Cols;
};

template <typename Param>
struct MatrixMultFixture : public ::testing::Test 
{
public:
    using DataType = typename Param::DataType;
    static constexpr int Rows = Param::Rows;
    static constexpr int Cols = Param::Cols;

    Matrix<DataType, Rows, Cols> testedA;
    Matrix<DataType, Rows, Cols> testedB;
    Matrix<DataType, Rows, Cols> testedC;

    boost::numeric::ublas::matrix<DataType> expectedA;
    boost::numeric::ublas::matrix<DataType> expectedB;
    boost::numeric::ublas::matrix<DataType> expectedC;

    MatrixMultFixture() : expectedA(Rows, Cols), expectedB(Rows, Cols), expectedC(Rows, Cols) {}

    void SetUp() override 
    {
        randomfillBoostMatrix(expectedA, static_cast<DataType>(0), static_cast<DataType>(100));
        randomfillBoostMatrix(expectedB, static_cast<DataType>(0), static_cast<DataType>(100));

        std::ranges::copy(expectedA.data(), testedA.data().begin());
        std::ranges::copy(expectedB.data(), testedB.data().begin());
    }
};

using TestedMatrixCombinations = ::testing::Types<
    MATRIX_DIMS_FOR_TYPE(float),
    MATRIX_DIMS_FOR_TYPE(double),
    MATRIX_DIMS_FOR_TYPE(uint32_t)
>;

TYPED_TEST_SUITE(MatrixMultFixture, TestedMatrixCombinations);

TYPED_TEST(MatrixMultFixture, WhenPerformingNaiveMatMultThenReturnCorrectResult)
{
    // TODO
}
