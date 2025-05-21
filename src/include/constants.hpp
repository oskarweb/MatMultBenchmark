#pragma once

#include <cstddef>
#include <cstdint>

namespace constants 
{
#ifdef _WIN32
    inline constexpr std::size_t getStackSize() { return STACK_SIZE; }
#else
    inline constexpr std::size_t getStackSize() { return 1024 * 1024; }
#endif

inline constexpr const uint32_t DEFAULT_MATRIX_ORDER = 128;
}