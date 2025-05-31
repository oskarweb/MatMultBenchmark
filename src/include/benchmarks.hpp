#pragma once

#include "constants.hpp"
#include "matrix.hpp"
#include "ocl_utils.hpp"
#include "thread_pool.hpp"
#include "utils.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <format>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string_view>

namespace Benchmarks 
{
inline constexpr const uint32_t DEFAULT_TASK_COUNT = 5;

class Runner 
{
public:
    template<uint32_t TaskCount = 1>
    double run() 
    {
        std::array<double, TaskCount> executionTimes;
        for (uint32_t i = TaskCount; i > 0; --i)
        {
            try 
            {
                this->setUp();
                const auto start{std::chrono::high_resolution_clock::now()};
                this->compute();
                const auto finish{std::chrono::high_resolution_clock::now()};
                executionTimes[TaskCount - i] = std::chrono::duration<double>{finish - start}.count();
            } catch (const std::runtime_error &e) 
            {
                std::cerr << e.what() << '\n';
            }
        }
        m_averageExecutionTime = std::accumulate(executionTimes.begin(), executionTimes.end(), 0.0) / static_cast<double>(TaskCount);
        return m_averageExecutionTime;
    }
protected:
    virtual void setUp() { return; }

    virtual void compute() { return; };

    const double &getAvgExecutionTime() { return m_averageExecutionTime; }

    double m_averageExecutionTime = 0;
};

template<typename Derived>
class Benchmark : protected Runner 
{
public: 
    static inline constexpr const uint32_t DEFAULT_TASK_COUNT = 10;

    Benchmark(std::string name) : m_name(name) {}    

    void measure() 
    {
        run<Derived::taskCount>();
        std::cout << describe();
    };
private:
    std::string describe() const 
    {
        std::string result;

        result += "Ran: " + m_name + "\n{\n";
        util::appendLabelValue(result, "Time(s) executed:", Derived::taskCount);
        util::appendLabelValue(result, "Avg. execution time:", std::format("{} s", m_averageExecutionTime));
        result += getExtraInfo() + "}\n";

        return result;
    }
protected: 
    virtual std::string getExtraInfo() const { return ""; }

    std::string m_name;
};

template<typename DataType, 
    uint32_t Rows = constants::DEFAULT_MATRIX_ORDER, 
    uint32_t Columns = constants::DEFAULT_MATRIX_ORDER,
    MultiplicationType MultType = MultiplicationType::Naive, 
    uint32_t TaskCount = DEFAULT_TASK_COUNT>
class MatMultBench : public Benchmark<MatMultBench<DataType, Rows, Columns, MultType, TaskCount>>  
{
public:
    static constexpr uint32_t taskCount = TaskCount;

    MatMultBench() : Benchmark<MatMultBench>("Matrix mult " + util::to_string(MultType)) {}  
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

    std::string getExtraInfo() const override 
    {
        std::string result;
        std::string dims = std::format("{} x {}", Columns, Rows);

        util::appendLabelValue(result, "Data type:", typeid(DataType).name());
        util::appendLabelValue(result, "Result dims:", dims);

        return result;
    }

    Matrix<DataType, Rows, Columns> m_matA;
    Matrix<DataType, Rows, Columns> m_matB;
    Matrix<DataType, Rows, Columns> m_matC;
};

template<typename DataType, uint32_t Rows, uint32_t Columns, uint32_t TaskCount>
class MatMultBench<DataType, Rows, Columns, MultiplicationType::NaiveOcl, TaskCount> : public Benchmark<MatMultBench<DataType, Rows, Columns, MultiplicationType::NaiveOcl, TaskCount>>  
{
public:
    static constexpr uint32_t taskCount = TaskCount;

    MatMultBench() : Benchmark<MatMultBench>("Matrix mult " + util::to_string(MultiplicationType::NaiveOcl)) {}  
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
        m_matC = m_matA.mult<MultiplicationType::NaiveOcl>(m_matB);
    }

    std::string getExtraInfo() const override 
    {
        std::string result;
        std::string dims = std::format("{} x {}", Columns, Rows);

        util::appendLabelValue(result, "Data type:", typeid(DataType).name());
        util::appendLabelValue(result, "Result dims:", dims);

        return result;
    }

    Matrix<DataType, Rows, Columns> m_matA;
    Matrix<DataType, Rows, Columns> m_matB;
    Matrix<DataType, Rows, Columns> m_matC;
};

template<typename T, 
    uint32_t Rows = constants::DEFAULT_MATRIX_ORDER, 
    uint32_t Columns = constants::DEFAULT_MATRIX_ORDER,
    MultiplicationType MultType = MultiplicationType::Naive, 
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



template <typename DataType, MultiplicationType... MultTypes>
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
        //MultiplicationType::Naive,
        //MultiplicationType::Simd,
        //MultiplicationType::MultithreadRow,
        //MultiplicationType::MultithreadElement,
        //MultiplicationType::MultithreadSimd,
        MultiplicationType::NaiveOcl>()), ...);
    return 0;
}

} // namespace Benchmarks