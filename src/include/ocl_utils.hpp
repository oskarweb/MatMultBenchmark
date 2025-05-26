#pragma once

#include "constants.hpp"

#include <CL/cl.h>

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

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

    ~Wrapper() { reset(); }

    void reset(T new_object = nullptr)
    {
        release();
        object = new_object;
    }

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

class Program
{
public:
    Program() = default;

    Program(std::filesystem::path kernelFilename) : m_kernelFilename(kernelFilename) {}

    Program(std::filesystem::path kernelFilename, const char *options)
    {
        m_kernelFilename = kernelFilename;
        cl_int err;

        std::unique_ptr<char[]> kernelSource = util::loadDataFromFile(constants::KERNELS_DIR / kernelFilename);
        const char *source[] = {kernelSource.get()};
        
        err = clGetPlatformIDs(1, &m_platform, nullptr);
        CHECK_CL_ERROR(err, "clGetPlatformIDs");
        
        err = clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_ALL, 1, &m_device, nullptr);
        CHECK_CL_ERROR(err, "clGetDeviceIDs");

        m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
        CHECK_CL_ERROR(err, "clCreateContext");

        m_program = clCreateProgramWithSource(m_context, 1, source, nullptr, &err);
        CHECK_CL_ERROR(err, "clCreateProgramWithSource");

        err = clBuildProgram(m_program, 1, &m_device, options, nullptr, nullptr);
        CHECK_CL_ERROR(err, "clBuildProgram");
    }

    Program(const char *kernelSource, const char *options)
    {
        cl_int err;
        
        err = clGetPlatformIDs(1, &m_platform, nullptr);
        CHECK_CL_ERROR(err, "clGetPlatformIDs");
        
        err = clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_ALL, 1, &m_device, nullptr);
        CHECK_CL_ERROR(err, "clGetDeviceIDs");

        m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
        CHECK_CL_ERROR(err, "clCreateContext");

        m_program = clCreateProgramWithSource(m_context, 1, &kernelSource, nullptr, &err);
        CHECK_CL_ERROR(err, "clCreateProgramWithSource");

        err = clBuildProgram(m_program, 1, &m_device, options, nullptr, nullptr);
        CHECK_CL_ERROR(err, "clBuildProgram");
    }

    void initialize()
    {
        cl_int err;
        
        err = clGetPlatformIDs(1, &m_platform, nullptr);
        CHECK_CL_ERROR(err, "clGetPlatformIDs");
        
        err = clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_ALL, 1, &m_device, nullptr);
        CHECK_CL_ERROR(err, "clGetDeviceIDs");

        m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
        CHECK_CL_ERROR(err, "clCreateContext");
    }

    void build(const char *options)
    {
        cl_int err;

        std::unique_ptr<char[]> kernelSource = util::loadDataFromFile(constants::KERNELS_DIR / m_kernelFilename);
        const char *source[] = {kernelSource.get()};

        m_program = clCreateProgramWithSource(m_context, 1, source, nullptr, &err);
        CHECK_CL_ERROR(err, "clCreateProgramWithSource");

        err = clBuildProgram(m_program, 1, &m_device, options, nullptr, nullptr);
        CHECK_CL_ERROR(err, "clBuildProgram");
    }

    virtual void compileBinary() 
    {
        cl_int err;

        size_t binarySize;
        err = clGetProgramInfo(m_program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &binarySize, nullptr);
        CHECK_CL_ERROR(err, "clGetProgramInfo");

        std::vector<unsigned char> binary(binarySize);
        unsigned char* binaryPtr = binary.data();

        err = clGetProgramInfo(m_program, CL_PROGRAM_BINARIES, sizeof(unsigned char*), &binaryPtr, nullptr);
        CHECK_CL_ERROR(err, "clGetProgramInfo");

        std::filesystem::path binaryName = m_kernelFilename.filename().replace_extension(".bin");

        util::writeDataToFile(constants::KERNELS_BIN_DIR / binaryName, binary.data(), binary.size());
    }

    virtual void loadFromBinary(std::filesystem::path binaryFilename) 
    {        
        cl_int binaryStatus;
        cl_int err;
        size_t binarySize{0};

        auto binaryPtr = util::loadDataFromBinaryFile(binaryFilename, &binarySize);

        const unsigned char *binaries[] = {binaryPtr.get()};

        m_program = clCreateProgramWithBinary(m_context, 1, &m_device, &binarySize, binaries, &binaryStatus, &err);
        CHECK_CL_ERROR(err, "clCreateProgramWithBinary");
        CHECK_CL_ERROR(binaryStatus, "Created binary");
        
        err = clBuildProgram(m_program, 1, &m_device, nullptr, nullptr, nullptr);
        CHECK_CL_ERROR(err, "clBuildProgram");
    }

    virtual void loadFromBinary() 
    {
        std::filesystem::path binaryName = m_kernelFilename.filename().replace_extension(".bin");
        loadFromBinary(constants::KERNELS_BIN_DIR / binaryName);
    }

    virtual void reset() 
    {
        m_program.reset();
        m_context.reset();
        m_device = nullptr;
        m_platform = nullptr;
    }

    virtual cl_program getProgram() { return m_program.get(); }
    virtual cl_context getContext() { return m_context.get(); }
protected:
    programWrapper m_program;
    contextWrapper m_context;
    cl_device_id m_device = nullptr;
    cl_platform_id m_platform = nullptr;
    std::filesystem::path m_kernelFilename;
};


class ProgramWithQueue : public Program
{
public:
    ProgramWithQueue() = default;

    ProgramWithQueue(std::filesystem::path kernelFilename) : Program(kernelFilename) {}

    ProgramWithQueue(std::filesystem::path kernelFilename, const char *options) : Program(kernelFilename, options) {}

    ProgramWithQueue(const char *kernelSource, const char *kernelName, const char *options)
    {
        cl_int err;
        
        err = clGetPlatformIDs(1, &m_platform, nullptr);
        CHECK_CL_ERROR(err, "clGetPlatformIDs");
        
        err = clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_ALL, 1, &m_device, nullptr);
        CHECK_CL_ERROR(err, "clGetDeviceIDs");

        m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
        CHECK_CL_ERROR(err, "clCreateContext");

        m_queue = clCreateCommandQueueWithProperties(m_context, m_device, 0, &err);
        CHECK_CL_ERROR(err, "clCreateCommandQueueWithProperties");

        m_program = clCreateProgramWithSource(m_context, 1, &kernelSource, nullptr, &err);
        CHECK_CL_ERROR(err, "clCreateProgramWithSource");

        err = clBuildProgram(m_program, 1, &m_device, options, nullptr, nullptr);
        CHECK_CL_ERROR(err, "clBuildProgram");

        m_kernel = clCreateKernel(m_program, kernelName, &err);
        CHECK_CL_ERROR(err, "clCreateKernel");
    } 

    void createQueueAndKernel(const char *kernelName) 
    {
        m_kernelName = kernelName;
        cl_int err;
        
        m_queue = clCreateCommandQueueWithProperties(m_context, m_device, 0, &err);
        CHECK_CL_ERROR(err, "clCreateCommandQueueWithProperties");

        m_kernel = clCreateKernel(m_program, kernelName, &err);
        CHECK_CL_ERROR(err, "clCreateKernel")
    }

    void reset() override 
    {
        m_kernel.reset();
        m_queue.reset();
        Program::reset();
    }

    cl_kernel getKernel() { return m_kernel.get(); }
    cl_command_queue getCmdQueue() { return m_queue.get(); }
private:
    kernelWrapper m_kernel;
    cmdQueueWrapper m_queue;
    std::string m_kernelName;
};

} // namespace oclUtil