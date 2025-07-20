#pragma once
#include <cstdint>
#include <random>

class GlobalRNG {
public:
    static std::mt19937& get() 
    {
        static std::mt19937 gen(s_seed);
        return gen;
    }

    static void init(unsigned int seed) 
    {
        s_seed = seed;
        get().seed(seed);
    }

    static unsigned int getSeed() { return s_seed; }
private:
    inline static unsigned int s_seed{ std::random_device{}() };
};