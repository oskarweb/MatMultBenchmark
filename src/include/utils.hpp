#pragma once

#include <algorithm>
#include <format>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>

template<typename DataType, uint32_t Rows, uint32_t Columns>
class Matrix;

enum class MultiplicationType;

namespace util 
{
inline constexpr std::string_view labelValueFormat = "\t{:<30}{}\n";

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


template<typename DataType, uint32_t Rows, uint32_t Columns>
std::ostream &operator<<(std::ostream &os, const Matrix<DataType, Rows, Columns> &m) 
{
    for (int i = 0; i < Rows; ++i) 
    {
        for (int j = 0; j < Columns; ++j) 
        {
            os << m(i, j) << " ";
        }
        if (i < (Rows - 1))
            os << "\n";
    }
    return os;
}

template <typename T, typename... Rest>
inline bool allEqual(const T& first, const Rest&... rest) 
{
    return ((first == rest) && ...);
}

template<typename labelType, typename valueType>
inline void appendLabelValue(std::string &str, const labelType &label, const valueType &value)
{
    std::format_to
    (
        std::back_inserter(str), 
        labelValueFormat,
        label,
        value
    );
}
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

std::string to_string(MultiplicationType type);
} // namespace util