#pragma once

#include "benchmark.hpp"

namespace Benchmarks
{

template<typename DataType, 
    uint32_t Rows = constants::DEFAULT_MATRIX_ORDER, 
    uint32_t Columns = constants::DEFAULT_MATRIX_ORDER,
    MatMultType MultType = MatMultType::Naive, 
    uint32_t TaskCount = DEFAULT_TASK_COUNT>
class MatMultBench : public Benchmark<MatMultBench<DataType, Rows, Columns, MultType, TaskCount>>  
{
public:
    static constexpr uint32_t taskCount = TaskCount;

    MatMultBench() : Benchmark<MatMultBench>("MatMult_" + util::toString(MultType)) {}  
protected:
    void setUp() override 
    {
        m_matA.randomFill();
        m_matB.randomFill();
    }

    void compute() override
    {
        m_matC = m_matA.mult<MultType>(m_matB);
    }

    virtual void injectOutputParams(boost::json::object &obj) const 
    {
        obj["data_type"] = typeid(DataType).name();
        obj["matrix_dims"] = std::format("{}x{}", Columns, Rows);
    }

    Matrix<DataType, Rows, Columns> m_matA;
    Matrix<DataType, Rows, Columns> m_matB;
    Matrix<DataType, Rows, Columns> m_matC;
};

template<typename DataType, uint32_t Rows, uint32_t Columns, uint32_t TaskCount>
class MatMultBench<DataType, Rows, Columns, MatMultType::NaiveOcl, TaskCount> : public Benchmark<MatMultBench<DataType, Rows, Columns, MatMultType::NaiveOcl, TaskCount>>  
{
public:
    static constexpr uint32_t taskCount = TaskCount;

    MatMultBench() : Benchmark<MatMultBench>("MatMult_" + util::toString(MatMultType::NaiveOcl)) {}  
protected:
    void setUp() override 
    {
        oclUtil::ProgramWithQueue program("mat_mult.cl");
        program.initialize();
        const std::string buildOptions = "-DT=" + oclUtil::getOpenCLTypeName<DataType>();
        program.build(buildOptions.c_str());
        program.saveBinary();
        m_matA.randomFill();
        m_matB.randomFill();
    }

    void compute() override
    {
        m_matC = m_matA.mult<MatMultType::NaiveOcl>(m_matB);
    }

    virtual void injectOutputParams(boost::json::object &obj) const 
    {
        obj["data_type"] = typeid(DataType).name();
        obj["matrix_dims"] = std::format("{}x{}", Columns, Rows);
    }

    Matrix<DataType, Rows, Columns> m_matA;
    Matrix<DataType, Rows, Columns> m_matB;
    Matrix<DataType, Rows, Columns> m_matC;
};

template<typename T, 
    uint32_t Rows = constants::DEFAULT_MATRIX_ORDER, 
    uint32_t Columns = constants::DEFAULT_MATRIX_ORDER,
    MatMultType MultType = MatMultType::Naive, 
    uint32_t TaskCount = DEFAULT_TASK_COUNT>
class ParCpuMatMultBench : public MatMultBench<T, Rows, Columns, MultType, TaskCount>
{
public: 
    ParCpuMatMultBench(std::string name) : 
        MatMultBench<T, Rows, Columns, MultType, TaskCount>(name),
        threadPool(std::max(std::thread::hardware_concurrency(), 4u)) {}  
protected:
    void compute() override 
    {
        for (int i = 0; i < 4; ++i) 
        {
            threadPool.enqueue([]{ std::cout << ""; });
        }
    }

    ThreadPool threadPool;
};

template <typename DataType, MatMultType... MultTypes>
int runMatrixMultTypes()
{
    ((MatMultBench<
        DataType,
        constants::DEFAULT_MATRIX_ORDER,
        constants::DEFAULT_MATRIX_ORDER,
        MultTypes,
        DEFAULT_TASK_COUNT>().measure()), ...);
    return 0;
}


template <typename... DataTypes>
int runAllMatrixMultTypesWithDataTypes() 
{
    ((runMatrixMultTypes<DataTypes,
        //MatMultType::Naive,
        //MatMultType::Simd,
        //MatMultType::MultithreadRow,
        //MatMultType::MultithreadElement,
        //MatMultType::MultithreadSimd,
        MatMultType::NaiveOcl>()), ...);
    return 0;
}

template <typename DataType>
int dispatchMultType(MatMultType mt) 
{
    if (MatMultType::Naive == mt)
        return runMatrixMultTypes<DataType, MatMultType::Naive>();
    if (MatMultType::Simd == mt)
        return runMatrixMultTypes<DataType, MatMultType::Simd>();
    if (MatMultType::MultithreadElement == mt)
        return runMatrixMultTypes<DataType, MatMultType::MultithreadElement>();
    if (MatMultType::MultithreadRow == mt)
        return runMatrixMultTypes<DataType, MatMultType::MultithreadRow>();
    if (MatMultType::MultithreadSimd == mt)
        return runMatrixMultTypes<DataType, MatMultType::MultithreadSimd>();
    if (MatMultType::NaiveOcl == mt)
        return runMatrixMultTypes<DataType, MatMultType::NaiveOcl>();
    throw std::runtime_error("Unknown mult type\n");
}

int dispatchMultDataType(MatMultType mt, MatMultDataType dt) 
{
    if (MatMultDataType::Int32 == dt)
        return dispatchMultType<int32_t>(mt);
    if (MatMultDataType::Uint32 == dt)
        return dispatchMultType<uint32_t>(mt);
    if (MatMultDataType::Float == dt)
        return dispatchMultType<float>(mt);
    if (MatMultDataType::Double == dt)
        return dispatchMultType<double>(mt);
    throw std::runtime_error("Unknown data type\n");
}

int dispatchMatMultBenchmarks(const std::vector<MatMultType> &mtV, const std::vector<MatMultDataType> &dtV) 
{
    std::cout << "[\n";
    for (size_t i = 0; i < mtV.size(); ++i) 
    {
        for (size_t j = 0; j < dtV.size(); ++j)
        {
            int res = 0;
            Benchmarks::dispatchMultDataType(mtV[i], dtV[j]);
            if (res != 0) return res;
            if (i == mtV.size() - 1 && j == dtV.size() - 1) 
                continue;
            std::cout << ',';
        }
    }
    std::cout << "]\n";

    return 0;
}

} // namespace Benchmarks