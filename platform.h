#pragma once


#if _MSC_VER
#define align_alloc( A, B ) _aligned_malloc( B, A )
#define align_free( A ) _aligned_free( A )
#else
#define align_alloc( A, B ) aligned_alloc( A, B )
#define align_free( A ) free( A )
#endif

