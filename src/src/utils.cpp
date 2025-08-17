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

template<>
MatMultType fromString(std::string_view str)
{
    const std::string strLower = toLower(str);
    if (strLower == "naive") { return MatMultType::Naive; }
    if (strLower == "simd") { return MatMultType::Simd; }
    if (strLower == "multithreadrow") { return MatMultType::MultithreadRow; }
    if (strLower == "multithreadelement") { return MatMultType::MultithreadElement; }
    if (strLower == "multithreadsimd") { return MatMultType::MultithreadSimd; }
    if (strLower == "naiveocl") { return MatMultType::NaiveOcl; }
    return MatMultType::Last;
}

template<>
MatMultDataType fromString(std::string_view str)
{
    const std::string strLower = toLower(str);
    if (strLower == "int32") { return MatMultDataType::Int32; }
    if (strLower == "uint32") { return MatMultDataType::Uint32; }
    if (strLower == "float") { return MatMultDataType::Float; }
    if (strLower == "doble") { return MatMultDataType::Double; }
    return MatMultDataType::Last;
}
}