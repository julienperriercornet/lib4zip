#pragma once


#include <cstdint>
#include <memory.h>
#include <string.h>


#ifdef AVX2
#if _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

static inline void aligned_memcpy(void* dst, void* src, size_t sz)
{
    uint8_t* source = (uint8_t*) src;
    uint8_t* destination = (uint8_t*) dst;
    uint8_t* end = (uint8_t*) src + sz;
    do {
        _mm256_store_si256( (__m256i*) destination, _mm256_load_si256((__m256i*) source) );
        source += 32;
        destination += 32;
    } while (source < end) ;
}

static inline void aligned_memset(void* dst, uint32_t elem, size_t sz)
{
    uint8_t* start = (uint8_t*) dst;
    uint8_t* end = ((uint8_t*) dst) + sz;
    __m256i element = _mm256_set1_epi32( elem );
    do {
        _mm256_store_si256( (__m256i*) start, element );
    } while ((start += 32) < end);
}
#else
static inline void aligned_memcpy(void* dst, void* src, size_t sz)
{
    memcpy(dst, src, sz);
}

static inline void aligned_memset(void* dst, uint32_t elem, size_t sz)
{
    memset(dst, elem, sz);
}
#endif

