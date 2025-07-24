#include "utils.hpp"
#include "matrix.hpp"

namespace util
{

std::string to_string(MatMultType type)
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

}