#pragma once

#include "matrix_benchmarks.hpp"

#include <boost/program_options.hpp>

#include <sstream>
#include <vector>

namespace MatBenchmarkProgOpts
{

template<typename Derived, typename T>
struct Option
{
    using value_type = typename T::value_type;

    static const T allowed;

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

    virtual std::string name() const = 0;
};

struct MatrixDims : Option<MatrixDims, decltype(Benchmarks::MatMultOrders)> 
{
    MatrixDims(const std::vector<int> &dimV) : opts(dimV) {}

    std::string name() const override { return "matrix_dims"; }

    std::vector<value_type> opts;
};

template<>
decltype(Benchmarks::MatMultOrders)
Option<MatrixDims, decltype(Benchmarks::MatMultOrders)>::allowed = Benchmarks::MatMultOrders;

void validate(boost::any &v, const std::vector<std::string> &values, MatrixDims *, int)
{
    using namespace boost::program_options;

    validators::check_first_occurrence(v);

    std::vector<int> parsed;
    for (auto& s : values) 
    {
        int val = std::stoi(s);
        if (std::find(MatrixDims::allowed.begin(), MatrixDims::allowed.end(), val) == MatrixDims::allowed.end()) 
        {
            throw validation_error(validation_error::invalid_option_value, s);
        }
        parsed.push_back(val);
    }
    v = MatrixDims{parsed};
}

} // namespace MatBenchmarkProgOpts