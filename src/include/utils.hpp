#pragma once

#include <boost/json.hpp>

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

enum class MatMultType;
enum class MatMultDataType;

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
    std::ifstream file(filename, std::ios::binary);

    if (not file) throw std::runtime_error("Failed to open file");

    std::stringstream ss;
    ss << file.rdbuf();
    std::string str = ss.str();
    
    size_t size = str.size() + 1;

    if (dataSize) 
        *dataSize = size;

    auto buffer = std::make_unique<char[]>(size);
    std::memcpy(buffer.get(), str.c_str(), size);

    file.close();
    if (not file) throw std::runtime_error("Failed to close file");

    return buffer;
}

inline std::unique_ptr<unsigned char[]> loadDataFromBinaryFile(std::filesystem::path filename, size_t *dataSize = nullptr) {
    std::ifstream file(filename, std::ios::binary);

    if (not file) throw std::runtime_error("Failed to open file");

    std::stringstream ss;
    ss << file.rdbuf();
    std::string str = ss.str();

    size_t size = str.size() + 1;

    if (dataSize) 
        *dataSize = size;

    auto buffer = std::make_unique<unsigned char[]>(size);
    std::memcpy(buffer.get(), str.c_str(), size);

    file.close();
    if (not file) throw std::runtime_error("Failed to close file");

    return buffer;
}

inline void writeDataToFile(std::filesystem::path filename, const unsigned char *data, size_t dataSize)
{
    std::ofstream file(filename, std::ios::binary);
    
    if (not file) throw std::runtime_error("Failed to open file");
    
    file.write(reinterpret_cast<const char*>(data), dataSize);
    if (not file) throw std::runtime_error("Failed to write data to file");

    file.close();
    if (not file) throw std::runtime_error("Failed to close file");
}

inline void prettyPrint(std::ostream &os, boost::json::value const &jv, std::string *indent = nullptr)
{
    std::string indent_;
    if(not indent)
        indent = &indent_;
    switch(jv.kind())
    {
        case boost::json::kind::object:
        {
            os << "{\n";
            indent->append(4, ' ');
            auto const& obj = jv.get_object();
            if(not obj.empty())
            {
                auto it = obj.begin();
                for(;;)
                {
                    os << *indent << boost::json::serialize(it->key()) << " : ";
                    prettyPrint(os, it->value(), indent);
                    if(++it == obj.end())
                        break;
                    os << ",\n";
                }
            }
            os << "\n";
            indent->resize(indent->size() - 4);
            os << *indent << "}";
            break;
        }
        case boost::json::kind::array:
        {
            os << "[\n";
            indent->append(4, ' ');
            auto const& arr = jv.get_array();
            if(not arr.empty())
            {
                auto it = arr.begin();
                for(;;)
                {
                    os << *indent;
                    prettyPrint( os, *it, indent);
                    if(++it == arr.end())
                        break;
                    os << ",\n";
                }
            }
            os << "\n";
            indent->resize(indent->size() - 4);
            os << *indent << "]";
            break;
        }
        case boost::json::kind::string:
        {
            os << boost::json::serialize(jv.get_string());
            break;
        }
        case boost::json::kind::uint64:
        case boost::json::kind::int64:
        case boost::json::kind::double_:
        {
            os << jv;
            break;
        }
        case boost::json::kind::bool_:
        {
            if(jv.get_bool())
                os << "true";
            else
                os << "false";
            break;
        }
        case boost::json::kind::null:
        {   
            os << "null";
            break;
        }
    }

    if(indent->empty())
        os << "\n";
}

inline void mergeJsonObjects(boost::json::object& dst, const boost::json::object& src)
{
    for (const auto& [key, value] : src) { dst.insert_or_assign(key, value); }
}

inline std::string toLower(std::string_view sv) 
{
    std::string result(sv);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string toString(MatMultType type);

template <typename Enum>
Enum fromString(std::string_view s) 
{
    static_assert(sizeof(Enum) == 0, "fromString not implemented");
}

template<typename T>
constexpr auto enumAll() 
requires requires { T::Last; }
{
    constexpr std::size_t N = static_cast<std::size_t>(T::Last) + 1;
    std::array<T, N> values{};
    for (std::size_t i = 0; i < N; ++i) {
        values[i] = static_cast<T>(i);
    }
    return values;
}
} // namespace util