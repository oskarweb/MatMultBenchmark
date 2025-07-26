#include "test_utils.hpp"

#include "constants.hpp"
#include "matrix.hpp"
#include "ocl_utils.hpp"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/operation.hpp>
#include <gtest/gtest.h>

#include <ranges>

#define MATRIX_DIMS_FOR_TYPE(T)                     \
    MatrixParams<T, 2, 2>,                          \
    MatrixParams<T, 4, 4>,                          \
    MatrixParams<T, 64, 64>

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

    boost::numeric::ublas::matrix<DataType> expectedA;
    boost::numeric::ublas::matrix<DataType> expectedB;
    boost::numeric::ublas::matrix<DataType> expectedResultMatrix;

    MatrixMultFixture() : expectedA(Rows, Cols), expectedB(Rows, Cols), expectedResultMatrix(Rows, Cols) {}

    void SetUp() override 
    {
        randomfillBoostMatrix(expectedA, static_cast<DataType>(0), static_cast<DataType>(100));
        randomfillBoostMatrix(expectedB, static_cast<DataType>(0), static_cast<DataType>(100));

        std::ranges::copy(expectedA.data(), testedA.data().begin());
        std::ranges::copy(expectedB.data(), testedB.data().begin());

        this->expectedResultMatrix = boost::numeric::ublas::prod(this->expectedA, this->expectedB);
    }

    template<MatMultType MultType>
    void run()
    {
        auto resultMatrix = this->testedA.mult<MultType>(this->testedB);

        for (size_t i = 0; i < this->expectedResultMatrix.size1(); ++i) 
        {
            for (size_t j = 0; j < this->expectedResultMatrix.size2(); ++j)
            {
                if constexpr(std::is_floating_point_v<DataType>) 
                {
                    ASSERT_NEAR(resultMatrix(i, j), this->expectedResultMatrix(i, j), 1);
                }
                else
                {
                    ASSERT_EQ(resultMatrix(i, j), this->expectedResultMatrix(i, j));
                }
            }
        }
    }
};

template <typename Param>
struct MatrixMultFixtureOcl : public MatrixMultFixture<Param>
{
    using DataType = typename Param::DataType;

    void SetUp() override 
    {
        oclUtil::ProgramWithQueue program("mat_mult.cl");
        program.initialize();
        const std::string buildOptions = "-DT=" + oclUtil::getOpenCLTypeName<DataType>();
        program.build(buildOptions.c_str());
        program.saveBinary();
        MatrixMultFixture<Param>::SetUp();
    }
};

using TestedMatrixDataTypeCombinations = ::testing::Types<
    MATRIX_DIMS_FOR_TYPE(int32_t),
    MATRIX_DIMS_FOR_TYPE(uint32_t),
    MATRIX_DIMS_FOR_TYPE(float),
    MATRIX_DIMS_FOR_TYPE(double)
>;

TYPED_TEST_SUITE(MatrixMultFixture, TestedMatrixDataTypeCombinations);

TYPED_TEST_SUITE(MatrixMultFixtureOcl, TestedMatrixDataTypeCombinations);

#define MAT_MULT_TYPED_TEST(CaseName, MultType)                             \
TYPED_TEST(CaseName, WhenPerforming##MultType##MultThenReturnCorrectResult) \
{                                                                           \
    this->run<MatMultType::MultType>();                                     \
}

MAT_MULT_TYPED_TEST(MatrixMultFixture, Naive);
MAT_MULT_TYPED_TEST(MatrixMultFixture, Simd);
MAT_MULT_TYPED_TEST(MatrixMultFixture, MultithreadElement);
MAT_MULT_TYPED_TEST(MatrixMultFixture, MultithreadRow);
MAT_MULT_TYPED_TEST(MatrixMultFixture, MultithreadSimd);
MAT_MULT_TYPED_TEST(MatrixMultFixtureOcl, NaiveOcl);