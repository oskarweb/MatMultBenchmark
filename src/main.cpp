#define CL_TARGET_OPENCL_VERSION 300

#include "benchmarks.hpp"
#include "ocl_utils.hpp"

#include <CL/cl.h>

#include <iostream>
#include <cstring>
#include <cassert>

#define CHECK_ERROR(err, msg) if (err != CL_SUCCESS) { std::cerr << msg << ": " << err << std::endl; exit(1); }


int main() 
{
    std::cout << "Stack size: " << STACK_SIZE << " bytes\n";
    const int N = 1024;
    std::vector<float> A(N, 1.0f), B(N, 2.0f), C(N);

    const char *kernelSource = R"(__kernel void vector_add(__global const float* A,
    __global const float* B,
    __global float* C) {
int id = get_global_id(0);
C[id] = A[id] + B[id];
})";

    oclUtil::ProgramWithQueue program(kernelSource, "vector_add");

    cl_int err;

    oclUtil::memWrapper bufA = clCreateBuffer(program.getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * N, A.data(), &err);
    oclUtil::memWrapper bufB = clCreateBuffer(program.getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * N, B.data(), &err);
    oclUtil::memWrapper bufC = clCreateBuffer(program.getContext(), CL_MEM_WRITE_ONLY, sizeof(float) * N, nullptr, &err);

    clSetKernelArg(program.getKernel(), 0, sizeof(cl_mem), &bufA);
    clSetKernelArg(program.getKernel(), 1, sizeof(cl_mem), &bufB);
    clSetKernelArg(program.getKernel(), 2, sizeof(cl_mem), &bufC);

    size_t globalSize = N;
    err = clEnqueueNDRangeKernel(program.getCmdQueue(), program.getKernel(), 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);
    CHECK_ERROR(err, "Failed to enqueue kernel");

    clEnqueueReadBuffer(program.getCmdQueue(), bufC, CL_TRUE, 0, sizeof(float) * N, C.data(), 0, nullptr, nullptr);

    for (int i = 0; i < 10; ++i)
        std::cout << "C[" << i << "] = " << C[i] << std::endl;

    std::cin.get();

    // int result = Benchmarks::runAllMatrixMultTypesWithDataTypes<uint32_t, float>();
    // std::cout << "RES 1: " << '\n';
    // std::cout << Res4 << '\n';
    // std::cout << "RES 2: " << '\n';
    // std::cout << Res5 << '\n';

    // assert(allEqual(Res1, Res2, Res3, Res4, Res5));

    // return result;
    return 0;
}