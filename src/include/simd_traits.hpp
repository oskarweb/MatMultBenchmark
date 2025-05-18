#pragma once

#include <immintrin.h>
#include <cstdint>

template <typename T>
struct SimdTraits;

template <>
struct SimdTraits<float> 
{
    using vec = __m256;
    static constexpr int width = 8;
    static vec load(const float *ptr) { return _mm256_loadu_ps(ptr); }
    static vec broadcast(float val) { return _mm256_set1_ps(val); }
    static vec add(vec a, vec b) { return _mm256_add_ps(a, b); }
    static vec mul(vec a, vec b) { return _mm256_mul_ps(a, b); }
    static void store(float* ptr, vec v) { _mm256_storeu_ps(ptr, v); }
};

template <>
struct SimdTraits<double> 
{
    using vec = __m256d;
    static constexpr int width = 4;
    static vec load(const double *ptr) { return _mm256_loadu_pd(ptr); }
    static vec broadcast(double val) { return _mm256_set1_pd(val); }
    static vec add(vec a, vec b) { return _mm256_add_pd(a, b); }
    static vec mul(vec a, vec b) { return _mm256_mul_pd(a, b); }
    static void store(double* ptr, vec v) { _mm256_storeu_pd(ptr, v); }
};

template <>
struct SimdTraits<int32_t> 
{
    using vec = __m256i;
    static constexpr int width = 8;
    static vec load(const int32_t *ptr) { return _mm256_loadu_si256((__m256i*)ptr); }
    static vec broadcast(int32_t val) { return _mm256_set1_epi32(val); }
    static vec add(vec a, vec b) { return _mm256_add_epi32(a, b); }
    static vec mul(vec a, vec b) { return _mm256_mullo_epi32(a, b); }
    static void store(int32_t *ptr, vec v) { _mm256_storeu_si256((__m256i*)ptr, v); }
};

template <>
struct SimdTraits<uint32_t> 
{
    using vec = __m256i;
    static constexpr int width = 8;
    static vec load(const uint32_t *ptr) { return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr)); }
    static vec broadcast(uint32_t val) { return _mm256_set1_epi32(static_cast<int32_t>(val)); }
    static vec add(vec a, vec b) { return _mm256_add_epi32(a, b); }
    static vec mul(vec a, vec b) { return _mm256_mullo_epi32(a, b); }
    static void store(uint32_t *ptr, vec v) { _mm256_storeu_si256(reinterpret_cast<__m256i*>(ptr), v); }
};