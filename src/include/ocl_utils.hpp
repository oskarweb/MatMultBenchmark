#pragma once

#define CL_TARGET_OPENCL_VERSION 300

#include "constants.hpp"
#include "utils.hpp"

#include <CL/cl.h>

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#define CHECK_CL_ERROR(err, func) if (err != CL_SUCCESS) { throw std::runtime_error(std::string(func) + " FAILED: " + oclUtil::clErrorToString(err)); }

namespace oclUtil
{

inline const char *clErrorToString(int errorCode) {
    switch (errorCode) 
    {
        case 0: return "CL_SUCCESS";
        case -1: return "CL_DEVICE_NOT_FOUND";
        case -2: return "CL_DEVICE_NOT_AVAILABLE";
        case -3: return "CL_COMPILER_NOT_AVAILABLE";
        case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case -5: return "CL_OUT_OF_RESOURCES";
        case -6: return "CL_OUT_OF_HOST_MEMORY";
        case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case -8: return "CL_MEM_COPY_OVERLAP";
        case -9: return "CL_IMAGE_FORMAT_MISMATCH";
        case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case -12: return "CL_MAP_FAILURE";
        case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case -15: return "CL_COMPILE_PROGRAM_FAILURE";
        case -16: return "CL_LINKER_NOT_AVAILABLE";
        case -17: return "CL_LINK_PROGRAM_FAILURE";
        case -18: return "CL_DEVICE_PARTITION_FAILED";
        case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
        case -30: return "CL_INVALID_VALUE";
        case -31: return "CL_INVALID_DEVICE_TYPE";
        case -32: return "CL_INVALID_PLATFORM";
        case -33: return "CL_INVALID_DEVICE";
        case -34: return "CL_INVALID_CONTEXT";
        case -35: return "CL_INVALID_QUEUE_PROPERTIES";
        case -36: return "CL_INVALID_COMMAND_QUEUE";
        case -37: return "CL_INVALID_HOST_PTR";
        case -38: return "CL_INVALID_MEM_OBJECT";
        case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case -40: return "CL_INVALID_IMAGE_SIZE";
        case -41: return "CL_INVALID_SAMPLER";
        case -42: return "CL_INVALID_BINARY";
        case -43: return "CL_INVALID_BUILD_OPTIONS";
        case -44: return "CL_INVALID_PROGRAM";
        case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
        case -46: return "CL_INVALID_KERNEL_NAME";
        case -47: return "CL_INVALID_KERNEL_DEFINITION";
        case -48: return "CL_INVALID_KERNEL";
        case -49: return "CL_INVALID_ARG_INDEX";
        case -50: return "CL_INVALID_ARG_VALUE";
        case -51: return "CL_INVALID_ARG_SIZE";
        case -52: return "CL_INVALID_KERNEL_ARGS";
        case -53: return "CL_INVALID_WORK_DIMENSION";
        case -54: return "CL_INVALID_WORK_GROUP_SIZE";
        case -55: return "CL_INVALID_WORK_ITEM_SIZE";
        case -56: return "CL_INVALID_GLOBAL_OFFSET";
        case -57: return "CL_INVALID_EVENT_WAIT_LIST";
        case -58: return "CL_INVALID_EVENT";
        case -59: return "CL_INVALID_OPERATION";
        case -60: return "CL_INVALID_GL_OBJECT";
        case -61: return "CL_INVALID_BUFFER_SIZE";
        case -62: return "CL_INVALID_MIP_LEVEL";
        case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
        case -64: return "CL_INVALID_PROPERTY";
        case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
        case -66: return "CL_INVALID_COMPILER_OPTIONS";
        case -67: return "CL_INVALID_LINKER_OPTIONS";
        case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";
        case -69: return "CL_INVALID_PIPE_SIZE";
        case -70: return "CL_INVALID_DEVICE_QUEUE";
        case -71: return "CL_INVALID_SPEC_ID";
        case -72: return "CL_MAX_SIZE_RESTRICTION_EXCEEDED";
        default: return "CL_UNKNOWN_ERROR";
    }
}

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
        if (not object) return;

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

    virtual void saveBinary() 
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

    ProgramWithQueue(const char *kernelSource, const char *options) : Program(kernelSource, options) {}

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
        m_kernelArgIndex = 0;
        Program::reset();
    }

    template<typename T>
    void setKernelArg(T &arg) 
    {
        clSetKernelArg(m_kernel.get(), m_kernelArgIndex++, sizeof(T), &arg);
    }

    cl_kernel getKernel() { return m_kernel.get(); }
    cl_command_queue getCmdQueue() { return m_queue.get(); }
protected:
    kernelWrapper m_kernel;
    cmdQueueWrapper m_queue;
    std::string m_kernelName;
    uint32_t m_kernelArgIndex = 0;
};

template<typename T>
inline std::string getOpenCLTypeName();

template<>
inline std::string getOpenCLTypeName<float>() { return "float"; }

template<>
inline std::string getOpenCLTypeName<double>() { return "double"; }

template<>
inline std::string getOpenCLTypeName<uint32_t>() { return "uint"; }

template<>
inline std::string getOpenCLTypeName<int>() { return "int"; }

template<>
inline std::string getOpenCLTypeName<unsigned short>() { return "ushort"; }

} // namespace oclUtil