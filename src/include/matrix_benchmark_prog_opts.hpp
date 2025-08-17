#pragma once

#include "matrix_benchmarks.hpp"
#include "utils.hpp"

#include <boost/program_options.hpp>

#include <sstream>
#include <vector>

namespace MatBenchmarkProgOpts
{

template <typename OptType>
void validate(boost::any& v, const std::vector<std::string>& values, OptType*, int)
{
    using namespace boost::program_options;
    using OptValueType = OptType::value_type;
    validators::check_first_occurrence(v);

    std::vector<OptValueType> parsed;
    for (auto& s : values) 
    {
        OptValueType val;
        if constexpr (std::is_enum_v<OptValueType>) 
        {
            val = fromString<OptValueType>(s);
        }
        else if constexpr (std::is_integral_v<OptValueType>)
        {
            val = std::stoi(s);
        } 

        if (std::find(OptType::allowed.begin(), OptType::allowed.end(), val) == OptType::allowed.end()) 
        {
            throw validation_error(validation_error::invalid_option_value, s);
        }

        parsed.push_back(val);
    }
    v = OptType{parsed};
}

template<typename Derived, typename T>
struct Option
{
    using value_type = typename T::value_type;

    static const T allowed;

    Option(const std::vector<value_type>& _opts) : opts(_opts) {}

    static std::string allowedStr(const std::string& sep = ", ") 
    {
        std::ostringstream oss;
        for (size_t i = 0; i < allowed.size(); ++i) 
        {
            oss << allowed[i];
            if (i + 1 < allowed.size())
                oss << sep;
        }
        return oss.str();
    }

    std::vector<value_type> opts; 
};

struct MatrixDimsOpt : Option<MatrixDimsOpt, decltype(Benchmarks::MatMultOrders)> 
{
    using Option::Option;

    inline static constexpr const char *name = "matrix_dims";
};

template<>
decltype(Benchmarks::MatMultOrders)
Option<MatrixDimsOpt, decltype(Benchmarks::MatMultOrders)>::allowed = Benchmarks::MatMultOrders;

struct MatMultTypeOpt : Option<MatMultTypeOpt, decltype(Benchmarks::MatMultOrders)> 
{
    using Option::Option;

    inline static constexpr const char *name = "mult_type";
};

template<>
const decltype(util::enumAll<MatMultType>())
Option<MatMultTypeOpt, decltype(util::enumAll<MatMultType>())>::allowed = util::enumAll<MatMultType>();

} // namespace MatBenchmarkProgOpts