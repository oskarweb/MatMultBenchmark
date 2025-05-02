#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <array>
#include <cassert>
#include <random>
#include <ranges>
#include <chrono>
#include <string_view>
#include <sstream>
#include <numeric>
#include <iomanip>

#define CHECK_ERROR(err, msg) if (err != CL_SUCCESS) { std::cerr << msg << ": " << err << std::endl; exit(1); }

template<typename T>
concept ElementIterable =
    std::ranges::range<T> &&
    requires(T t)
    {
        { t.size() } -> std::same_as<std::size_t>;
    } &&
    std::ranges::range<std::ranges::range_value_t<T>> &&
    requires(std::ranges::range_value_t<T> elem)
    {
        { elem.size() } -> std::same_as<std::size_t>;
    };

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

    virtual void compute() {
        return;
    };

    const double &getAvgExecutionTime() 
    {
        return m_averageExecutionTime;
    }

    double m_averageExecutionTime = 0;
};

class Info {
public:
    constexpr Info(std::string_view name) : m_name(name) {}
    virtual std::string getDescription() const {
        return "None";
    }
protected:
    std::string_view m_name;
};

template<typename Derived>
class Benchmark : protected Runner, protected Info {
public: 
    Benchmark(std::string_view name) : Info(name) {}    

    void measure() {
        run<Derived::taskCount>();
        describe();
    };
private:
    void describe() {
        std::cout << getDescription();
    }

    std::string getDescription() const override {
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
};

template<typename T, uint32_t Order = 2, uint32_t TaskCount = 1>
class SeqCpuMatMultBench : public Benchmark<SeqCpuMatMultBench<T, Order, TaskCount>>
{
public: 
    static constexpr uint32_t taskCount = TaskCount;
    SeqCpuMatMultBench(std::string_view name) : Benchmark<SeqCpuMatMultBench>(name) {}  
protected:
    void setUp() override 
    {
        m_matA = genRandomMatrix<T, Order>();
        m_matB = genRandomMatrix<T, Order>();
    }

    void compute() override 
    {
        [[maybe_unused]] auto resultMatrix = matMult(m_matA, m_matB);
    }

    std::array<std::array<T, Order>, Order> m_matA;
    std::array<std::array<T, Order>, Order> m_matB;
};

std::ostream& operator<<(std::ostream& os, const Info& info) {
    return os << info.getDescription() << '\n';
}

int main() {
//     const int N = 1024;
//     std::vector<float> A(N, 1.0f), B(N, 2.0f), C(N);

//     cl_platform_id platform;
//     cl_device_id device;
//     cl_int err;

//     err = clGetPlatformIDs(1, &platform, nullptr);
//     CHECK_ERROR(err, "Failed to get platform");

//     err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
//     CHECK_ERROR(err, "Failed to get device");

//     cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
//     CHECK_ERROR(err, "Failed to create context");

//     cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
//     CHECK_ERROR(err, "Failed to create queue");

//     cl_mem bufA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * N, A.data(), &err);
//     cl_mem bufB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * N, B.data(), &err);
//     cl_mem bufC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * N, nullptr, &err);

//     const char *kernelSource = R"(__kernel void vector_add(__global const float* A,
//                          __global const float* B,
//                          __global float* C) {
//     int id = get_global_id(0);
//     C[id] = A[id] + B[id];
// })";

//     cl_program program = clCreateProgramWithSource(context, 1, &kernelSource, nullptr, &err);
//     err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    
//     if (err != CL_SUCCESS) {
//         size_t logSize;
//         clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
//         std::vector<char> log(logSize);
//         clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
//         std::cerr << "Build log:\n" << log.data() << std::endl;
//         return 1;
//     }

//     cl_kernel kernel = clCreateKernel(program, "vector_add", &err);
//     CHECK_ERROR(err, "Failed to create kernel");

//     // Set arguments
//     clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
//     clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
//     clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufC);

//     // Launch kernel
//     size_t globalSize = N;
//     err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);
//     CHECK_ERROR(err, "Failed to enqueue kernel");

//     // Read result
//     clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, sizeof(float) * N, C.data(), 0, nullptr, nullptr);

//     // Print some results
//     for (int i = 0; i < 10; ++i)
//         std::cout << "C[" << i << "] = " << C[i] << std::endl;

//     // Cleanup
//     clReleaseMemObject(bufA);
//     clReleaseMemObject(bufB);
//     clReleaseMemObject(bufC);
//     clReleaseKernel(kernel);
//     clReleaseProgram(program);
//     clReleaseCommandQueue(queue);
//     clReleaseContext(context);

//     std::cin.get();
    SeqCpuMatMultBench<uint64_t, 100u, 10u> cpuSeqBench1("CpuSeqBench(1)");
    cpuSeqBench1.measure();

    return 0;
}