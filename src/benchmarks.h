#pragma once

#include "matrix.h"
#include "thread_pool.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string_view>

class Runner 
{
public:
    template<uint32_t TaskCount = 1>
    double run() 
    {
        std::array<double, TaskCount> executionTimes;
        for (uint32_t i = TaskCount; i > 0; --i)
        {
            this->setUp();
            const auto start{std::chrono::high_resolution_clock::now()};
            this->compute();
            const auto finish{std::chrono::high_resolution_clock::now()};
            executionTimes[TaskCount - i] = std::chrono::duration<double>{finish - start}.count();
        }
        m_averageExecutionTime = std::accumulate(executionTimes.begin(), executionTimes.end(), 0.0) / static_cast<double>(TaskCount);
        return m_averageExecutionTime;
    }
protected:
    virtual void setUp() 
    {
        return;
    }

    virtual void compute() 
    {
        return;
    };

    const double &getAvgExecutionTime() 
    {
        return m_averageExecutionTime;
    }

    double m_averageExecutionTime = 0;
};

template<typename Derived>
class Benchmark : protected Runner {
public: 
    Benchmark(std::string_view name) : m_name(name) {}    

    void measure() {
        run<Derived::taskCount>();
        std::cout << describe();
    };
private:
    std::string describe() const {
        std::stringstream os;
        os << m_name << '\n';
        os << "{\n";
        os << std::left << std::setw(30) << "\ttime(s) executed: " << Derived::taskCount << '\n';
        os << std::left << std::setw(30) << "\taverage execution time: " << m_averageExecutionTime << "s\n";
        os << getExtraInfo();
        os << "}\n";
        return os.str();
    }
protected: 
    virtual std::string getExtraInfo() const {
        return "";
    }

    std::string_view m_name;
};

template<typename T, uint32_t Order = 2, uint32_t TaskCount = 1>
class MatMultBench : public Benchmark<MatMultBench<T, Order, TaskCount>>  {
public:
    static constexpr uint32_t taskCount = TaskCount;
protected:
    MatMultBench(std::string_view name) : Benchmark<MatMultBench>(name) {}  
    
    void setUp() override 
    {
        m_matA = genRandomMatrix<T, Order>();
        m_matB = genRandomMatrix<T, Order>();
    }

    std::array<std::array<T, Order>, Order> m_matA;
    std::array<std::array<T, Order>, Order> m_matB;
    std::array<std::array<T, Order>, Order> m_matC;
};

template<typename T, uint32_t Order = 2, uint32_t TaskCount = 1>
class SeqCpuMatMultBench : public MatMultBench<T, Order, TaskCount>
{
public: 
    SeqCpuMatMultBench(std::string_view name) : MatMultBench<T, Order, TaskCount>(name) {}  
protected:
    void compute() override 
    {
        this->m_matC = matMult(this->m_matA, this->m_matB);
    }
};

template<typename T, uint32_t Order = 2, uint32_t TaskCount = 1>
class ParCpuMatMultBench : public MatMultBench<T, Order, TaskCount>
{
public: 
    ParCpuMatMultBench(std::string_view name) : 
        MatMultBench<T, Order, TaskCount>(name),
        threadPool(std::max(std::jthread::hardware_concurrency(), 4u)) {}  
protected:
    void compute() override 
    {
        for (int i = 0; i < 4; ++i) {
            threadPool.enqueue([]{ std::cout << ""; });
        }
    }

    ThreadPool threadPool;
};