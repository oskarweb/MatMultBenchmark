#pragma once

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>

template<typename DataType, int Rows, int Columns>
class Matrix;

enum class MultiplicationType;

template<typename DataType, int Rows, int Columns>
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

inline std::unique_ptr<char[]> loadDataFromFile(std::filesystem::path filename, size_t *dataSize = nullptr) {
    if (!std::filesystem::exists(filename)) {
        std::cerr << "File does not exist: " << filename << std::endl;
    }
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Failed to open file");

    std::stringstream ss;
    ss << file.rdbuf();
    std::string str = ss.str();
    
    size_t size = str.size() + 1;

    if (dataSize) 
        *dataSize = size;

    auto buffer = std::make_unique<char[]>(size);
    std::memcpy(buffer.get(), str.c_str(), size);

    file.close();
    if (!file) throw std::runtime_error("Failed to close file");

    return buffer;
}

inline std::unique_ptr<unsigned char[]> loadDataFromBinaryFile(std::filesystem::path filename, size_t *dataSize = nullptr) {
    std::ifstream file(filename, std::ios::binary);
    if (!std::filesystem::exists(filename)) {
        std::cerr << "File does not exist: " << filename << std::endl;
    }
    if (!file) throw std::runtime_error("Failed to open file");

    std::stringstream ss;
    ss << file.rdbuf();
    std::string str = ss.str();

    size_t size = str.size() + 1;

    if (dataSize) 
        *dataSize = size;

    auto buffer = std::make_unique<unsigned char[]>(size);
    std::memcpy(buffer.get(), str.c_str(), size);

    file.close();
    if (!file) throw std::runtime_error("Failed to close file");

    return buffer;
}

inline void writeDataToFile(std::filesystem::path filename, const unsigned char *data, size_t dataSize)
{
    std::ofstream file(filename, std::ios::binary);
    // std::cerr << "Current working directory: " << std::filesystem::current_path() << std::endl;
    if (!std::filesystem::exists(filename)) {
        std::cerr << "File does not exist: " << filename << std::endl;
    }
    if (!file) throw std::runtime_error("Failed to open file");
    
    file.write(reinterpret_cast<const char*>(data), dataSize);
    if (!file) throw std::runtime_error("Failed to write data to file");

    file.close();
    if (!file) throw std::runtime_error("Failed to close file");
}

std::string to_string(MultiplicationType type);
} // namespace util