#pragma once

#include "constants.hpp"
#include "matrix.hpp"
#include "ocl_utils.hpp"
#include "thread_pool.hpp"
#include "utils.hpp"

#include <boost/json.hpp>

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
    template<uint32_t TaskCount = DEFAULT_TASK_COUNT>
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
    Benchmark(std::string name) : m_name(name) {}    

    void measure() 
    {
        run<Derived::taskCount>();
        util::prettyPrint(std::cout, getOutput());
    };
private:
    boost::json::object getOutput() const 
    {
        boost::json::object output;
        output["name"] = m_name;
        output["times_executed"] = Derived::taskCount;
        output["avg_execution_time_seconds"] = m_averageExecutionTime;
        injectOutputParams(output);

        return output;
    }
protected: 
    virtual void injectOutputParams(boost::json::object &) const { return; }

    std::string m_name;
};

} // namespace Benchmarks