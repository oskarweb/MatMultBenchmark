#pragma once

#include <CL/cl.h>

#include <stdexcept>
#include <string>

namespace oclUtil
{
#define CHECK_CL_ERROR(err, func) if (err != CL_SUCCESS) { throw std::runtime_error(std::string(func) + " FAILED: " + std::to_string(err)); }

template <typename T> 
using ReleaseType = cl_int CL_API_CALL(T);

template <typename T, ReleaseType<T> Release>
class Wrapper {
public:
    Wrapper() = default;

    Wrapper(T object): object(object) {}

    Wrapper &operator=(T rhs)
    {
        object = rhs;
        return *this;
    }

    ~Wrapper() { release(); }

    operator T() const { return object; }
    T get() const { return object; }
    T *operator&() { return &object; }
private:
    static_assert(std::is_pointer<T>::value, "T should be a pointer type.");

    T object = nullptr;

    void release()
    {
        if (!object) return;

        auto err = Release(object);
        CHECK_CL_ERROR(err, "clRelease*()");
    }
};

using contextWrapper = Wrapper<cl_context, clReleaseContext>;

using programWrapper = Wrapper<cl_program, clReleaseProgram>;

using kernelWrapper = Wrapper<cl_kernel, clReleaseKernel>;

using memWrapper = Wrapper<cl_mem, clReleaseMemObject>;

using cmdQueueWrapper = Wrapper<cl_command_queue, clReleaseCommandQueue>;

class ProgramWithQueue 
{
public:
    ProgramWithQueue(const char *kernelSource, const char *kernelName)
    {
        cl_int err;
        
        err = clGetPlatformIDs(1, &platform, nullptr);
        CHECK_CL_ERROR(err, "clGetPlatformIDs");
        
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, nullptr);
        CHECK_CL_ERROR(err, "clGetDeviceIDs");

        context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
        CHECK_CL_ERROR(err, "clCreateContext");

        queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
        CHECK_CL_ERROR(err, "clCreateCommandQueueWithProperties");

        program = clCreateProgramWithSource(context, 1, &kernelSource, nullptr, &err);
        CHECK_CL_ERROR(err, "clCreateProgramWithSource");

        err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
        CHECK_CL_ERROR(err, "clBuildProgram");

        kernel = clCreateKernel(program, kernelName, &err);
        CHECK_CL_ERROR(err, "clCreateKernel");
    }

    cl_kernel getKernel() { return kernel.get(); }
    cl_command_queue getCmdQueue() { return queue.get(); }
    cl_context getContext() { return context.get(); }
private:
    kernelWrapper kernel;
    programWrapper program;
    cmdQueueWrapper queue;
    contextWrapper context;
    cl_device_id device;
    cl_platform_id platform;
};

} // namespace oclUtil