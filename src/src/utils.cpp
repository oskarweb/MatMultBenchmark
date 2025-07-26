#include "utils.hpp"
#include "matrix.hpp"

#include <ranges>
#include <algorithm>

namespace util
{

std::string toString(MatMultType type)
{
    switch (type) 
    {
        case MatMultType::Naive:             
            return "Naive";
        case MatMultType::Simd:              
            return "Simd";
        case MatMultType::MultithreadRow:    
            return "MultithreadRow";
        case MatMultType::MultithreadElement:
            return "MultithreadElement";
        case MatMultType::MultithreadSimd:   
            return "MultithreadSimd";
        case MatMultType::NaiveOcl:   
            return "NaiveOcl";
        default:                                    
            return "Unknown";
    }
}

MatMultType toMultType(std::string_view str)
{
    const std::string strLower = toLower(str);
    if (strLower == "naive") { return MatMultType::Naive; }
    if (strLower == "simd") { return MatMultType::Simd; }
    if (strLower == "multithreadrow") { return MatMultType::MultithreadRow; }
    if (strLower == "multithreadelement") { return MatMultType::MultithreadElement; }
    if (strLower == "multithreadsimd") { return MatMultType::MultithreadSimd; }
    if (strLower == "naiveocl") { return MatMultType::NaiveOcl; }
    return MatMultType::Unknown;
}

}