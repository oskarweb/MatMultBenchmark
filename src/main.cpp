#define CL_TARGET_OPENCL_VERSION 300

#include "benchmarks.h"

#include <CL/cl.h>

#include <iostream>
#include <cstring>
#include <cassert>

#define CHECK_ERROR(err, msg) if (err != CL_SUCCESS) { std::cerr << msg << ": " << err << std::endl; exit(1); }


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
    ParCpuMatMultBench<uint64_t, 100u, 1u> cpuParBench1("CpuParBench(1)");
    cpuParBench1.measure();

    Matrix<uint32_t, 64, 64> a{};
    Matrix<uint32_t, 64, 64> b{};

    a.randomFill(0, 1);
    b.randomFill(0, 1);

    // std::cout << a << '\n';
    // std::cout << b << '\n';

    Matrix Res1 = a.mult<MultType::Naive>(b);
    Matrix Res2 = a.mult<MultType::Simd>(b);
    Matrix Res3 = a.mult<MultType::MultithreadRow>(b);
    Matrix Res4 = a.mult<MultType::MultithreadElement>(b);
    Matrix Res5 = a.mult<MultType::MultithreadSimd>(b);
    // std::cout << "RES 1: " << '\n';
    // std::cout << Res4 << '\n';
    // std::cout << "RES 2: " << '\n';
    // std::cout << Res5 << '\n';

    assert(allEqual(Res1, Res2, Res3, Res4, Res5));

    return 0;
}