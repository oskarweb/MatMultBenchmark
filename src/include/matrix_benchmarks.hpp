#pragma once

#include "benchmark.hpp"

namespace Benchmarks
{

template<typename DataType, 
    uint32_t Rows = constants::DEFAULT_MATRIX_ORDER, 
    uint32_t Columns = constants::DEFAULT_MATRIX_ORDER,
    MatMultType MultType = MatMultType::Naive, 
    uint32_t TaskCount = DEFAULT_TASK_COUNT>
class MatMultBenchmark : public Benchmark<MatMultBenchmark<DataType, Rows, Columns, MultType, TaskCount>>  
{
public:
    static constexpr uint32_t taskCount = TaskCount;

    MatMultBenchmark() : Benchmark<MatMultBenchmark>("MatMult_" + util::toString(MultType)) {}  
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
class MatMultBenchmark<DataType, Rows, Columns, MatMultType::NaiveOcl, TaskCount> 
    : public Benchmark<MatMultBenchmark<DataType, Rows, Columns, MatMultType::NaiveOcl, TaskCount>>  
{
public:
    static constexpr uint32_t taskCount = TaskCount;

    MatMultBenchmark() : Benchmark<MatMultBenchmark>("MatMult_" + util::toString(MatMultType::NaiveOcl)) {}  
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
class ParCpuMatMultBenchmark : public MatMultBenchmark<T, Rows, Columns, MultType, TaskCount>
{
public: 
    ParCpuMatMultBenchmark(std::string name) : 
        MatMultBenchmark<T, Rows, Columns, MultType, TaskCount>(name),
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

template <int Order, typename DataType, MatMultType... MultTypes>
int runMatrixMultTypes()
{
    ((MatMultBenchmark<
        DataType,
        Order,
        Order,
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

template <typename DataType, MatMultType type>
int dispatchMatOrder(int order)
{
    if (2 == order)
        return runMatrixMultTypes<2, DataType, type>();
    if (128 == order)
        return runMatrixMultTypes<128, DataType, type>();
    if (256 == order)
        return runMatrixMultTypes<256, DataType, type>();
    if (512 == order)
        return runMatrixMultTypes<512, DataType, type>();
    if (1024 == order)
        return runMatrixMultTypes<1024, DataType, type>();
    throw std::runtime_error("Unsupported mat mult order\n");
}

template <typename DataType>
int dispatchMultType(int order, MatMultType mt) 
{
    if (MatMultType::Naive == mt)
        return dispatchMatOrder<DataType, MatMultType::Naive>(order);
    if (MatMultType::Simd == mt)
        return dispatchMatOrder<DataType, MatMultType::Simd>(order);
    if (MatMultType::MultithreadElement == mt)
        return dispatchMatOrder<DataType, MatMultType::MultithreadElement>(order);
    if (MatMultType::MultithreadRow == mt)
        return dispatchMatOrder<DataType, MatMultType::MultithreadRow>(order);
    if (MatMultType::MultithreadSimd == mt)
        return dispatchMatOrder<DataType, MatMultType::MultithreadSimd>(order);
    if (MatMultType::NaiveOcl == mt)
        return dispatchMatOrder<DataType, MatMultType::NaiveOcl>(order);
    throw std::runtime_error("Unsupported mat mult type\n");
}

int dispatchMultDataType(int order, MatMultType multType, MatMultDataType dataType) 
{
    if (MatMultDataType::Int32 == dataType)
        return dispatchMultType<int32_t>(order, multType);
    if (MatMultDataType::Uint32 == dataType)
        return dispatchMultType<uint32_t>(order, multType);
    if (MatMultDataType::Float == dataType)
        return dispatchMultType<float>(order, multType);
    if (MatMultDataType::Double == dataType)
        return dispatchMultType<double>(order, multType);
    throw std::runtime_error("Unsupported mat mult data type\n");
}

int dispatchMatMultBenchmarks(const std::vector<int> &orderV, const std::vector<MatMultType> &multTypeV, const std::vector<MatMultDataType> &dataTypeV) 
{
    int result{-1};
    
    for (auto &order : orderV)
    {
        for (auto &multType : multTypeV) 
        {
            for (auto &dataType : dataTypeV) 
            {
                result = Benchmarks::dispatchMultDataType(order, multType, dataType);
                if (result != 0) return result;
            }
        }
    }

    return result;
}

} // namespace Benchmarks