#include "utils.hpp"
#include "matrix.hpp"

namespace util
{

std::string to_string(MultiplicationType type)
{
    switch (type) 
    {
        case MultiplicationType::Naive:             
            return "Naive";
        case MultiplicationType::Simd:              
            return "Simd";
        case MultiplicationType::MultithreadRow:    
            return "MultithreadRow";
        case MultiplicationType::MultithreadElement:
            return "MultithreadElement";
        case MultiplicationType::MultithreadSimd:   
            return "MultithreadSimd";
        default:                                    
            return "Unknown";
    }
}

}