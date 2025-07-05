#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>

namespace constants 
{
#ifdef _WIN32
    inline constexpr std::size_t getStackSize() { return STACK_SIZE; }
#else
    inline constexpr std::size_t getStackSize() { return 1024 * 1024; }
#endif

inline constexpr const int DEFAULT_MATRIX_ORDER = 128;

inline const std::filesystem::path KERNELS_DIR = std::filesystem::path("..") / "kernels";
inline const std::filesystem::path KERNELS_BIN_DIR = KERNELS_DIR / "bin";
}