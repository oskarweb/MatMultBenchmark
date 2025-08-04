#include "matrix_benchmarks.hpp"
#include "ocl_utils.hpp"
#include "utils.hpp"

#include <boost/program_options.hpp>
#include <CL/cl.h>

#include <iostream>
#include <cstring>
#include <cassert>

#define CHECK_ERROR(err, msg) if (err != CL_SUCCESS) { std::cerr << msg << ": " << err << std::endl; exit(1); }

namespace boost_po = boost::program_options;

int main(int argc, char *argv[]) 
{
    std::cout << "Stack size: " << constants::getStackSize() << " bytes\n";
    const int N = 1024;
    std::vector<float> A(N, 1.0f), B(N, 2.0f), C(N);

//     const char *kernelSource = R"(__kernel void vector_add(__global const float* A,
//     __global const float* B,
//     __global float* C) {
// int id = get_global_id(0);
// C[id] = A[id] + B[id];
// })";

    oclUtil::ProgramWithQueue program("test.cl");
    program.initialize();
    program.build(nullptr);
    program.saveBinary();
    program.reset();
    program.initialize();
    program.loadFromBinary();
    program.createQueueAndKernel("vector_add");

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

    // std::cin.get();

    // Benchmarks::runAllMatrixMultTypesWithDataTypes<uint32_t>();
    // std::cout << "RES 1: " << '\n';
    // std::cout << Res4 << '\n';
    // std::cout << "RES 2: " << '\n';
    // std::cout << Res5 << '\n';

    // assert(allEqual(Res1, Res2, Res3, Res4, Res5));

    // Matrix<uint32_t, 128, 128> mat1{};
    // mat1.randomFill();
    // Matrix<uint32_t, 128, 128> mat2{};
    // mat2.randomFill();
    // auto res1 = mat1.mult<MatMultType::Naive>(mat2);
    // // std::cout << "Res1: " << '\n' << res1;
    // auto res2 = mat1.mult<MatMultType::Simd>(mat2);
    // // std::cout << "\nRes2: " << '\n' << res2;
    // //auto res3 = mat1.mult<MatMultType::MultithreadElement>(mat2);
    // // std::cout << "\nRes3: " << '\n' << res3;
    // //auto res4 = mat1.mult<MatMultType::MultithreadRow>(mat2);
    // // std::cout << "\nRes4: " << '\n' << res4;
    // auto res5 = mat1.mult<MatMultType::MultithreadSimd>(mat2);
    // // std::cout << "\nRes5: " << '\n' << res5;
    // auto res6 = mat1.mult<MatMultType::NaiveOcl>(mat2);
    // std::cout << "\nRes6: " << '\n' << res6;
    try {
        boost_po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("compression", boost_po::value<double>(), "set compression level")
        ;

        boost_po::variables_map vm;        
        boost_po::store(boost_po::parse_command_line(argc, argv, desc), vm);
        boost_po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        if (vm.count("compression")) {
            std::cout << "Compression level was set to " 
                 << vm["compression"].as<double>() << ".\n";
        } else {
            std::cout << "Compression level was not set.\n";
        }
    }
    catch(std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        std::cerr << "Exception of unknown type!\n";
    }

    std::vector<int> matrixOrdersToDispatch = { 128, 256 };
    std::vector<MatMultType> multTypesToDispatch = { MatMultType::Naive, MatMultType::NaiveOcl };
    std::vector<MatMultDataType> dataTypesToDispatch = { MatMultDataType::Float, MatMultDataType::Double };

    return Benchmarks::dispatchMatMultBenchmarks(matrixOrdersToDispatch,multTypesToDispatch, dataTypesToDispatch);


    // //assert(util::allEqual(res1, res2, res5, res6));

    // // return result;
    // return 0;
}