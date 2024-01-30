#pragma once


#include <cassert>


//#define LZAAHE_SORT_STATS
//#define LZAAHE_SORT_DEBUG


static inline void lzaahe_swapDictItem( uint32_t& di1, uint32_t& di2 )
{
    uint32_t tmp = di1;
    di1 = di2;
    di2 = tmp;
}


static bool lzaahe_verifyArray( uint32_t* dictionnary, uint32_t size )
{
    uint32_t j = 0;

    while (j < size-1 && dictionnary[j] >= dictionnary[j+1])
        j++;

    return j == size-1;
}


#ifdef LZAAHE_SORT_STATS
static uint64_t lzaahe_iteration_ctr;
static uint64_t lzaahe_bubbleSort2_calls;
#endif


static void lzaahe_bubbleSort( uint32_t* dictionnary, uint32_t size )
{
    uint32_t j = 0;

#ifdef LZAAHE_SORT_STATS
    lzaahe_bubbleSort2_calls++;
#endif

    do {
        // Search the first non-sorted indice
        while (j < size-1 && dictionnary[j] >= dictionnary[j+1])
        {
            j++;

#ifdef LZAAHE_SORT_STATS
            lzaahe_iteration_ctr++;
#endif
        }

        // Did we stop before the end? => the array isn't sorted
        if (j < size-1)
        {
            bool nextjfound = false;
            uint32_t nextj = 0;
            uint32_t i=j;

            while (i < size-1 && dictionnary[i] < dictionnary[i+1])
            {
                lzaahe_swapDictItem( dictionnary[i], dictionnary[i+1] );

                if (i > 0 && dictionnary[i-1] < dictionnary[i] && !nextjfound)
                {
                    nextj = i-1;
                    nextjfound = true;
                }

                i++;
#ifdef LZAAHE_SORT_STATS
                lzaahe_iteration_ctr++;
#endif
            }

            if (nextjfound) j = nextj;
            else j = i;
        }

    } while (j < size-1);

#ifdef LZAAHE_SORT_DEBUG
    assert(lzaahe_verifyArray( dictionnary, size ));
#endif
}


static void lzaahe_insertionSingleSort( uint32_t* dictionnary, uint32_t ind, uint32_t size )
{
    uint32_t i = ind;

    while (i < size-1 && dictionnary[i] < dictionnary[i+1])
    {
        lzaahe_swapDictItem( dictionnary[i], dictionnary[i+1] );
        i++;
    }
}


#if 0
static inline uint32_t lzaahe_probaGamble( uint32_t sum_1, uint32_t sum )
{
    if (sum_1 == 0) return 0;
    uint32_t p = (uint32_t) (((((uint64_t) sum_1) << ARITH_PRECISION) - (sum_1 << 1)) / sum) + 1;
    return p;
}


static void lzaahe_updateProbaTables( uint32_t* dict, uint32_t** tables, uint32_t** tmp )
{
    memset( tables[0], 0, 256*sizeof(uint32_t) );
    memset( tmp[0], 0, 256*sizeof(uint32_t) );

    for (uint32_t i=0; i<256; i++)
    {
        uint32_t currentstat = dict[i] >> 8;

        tmp[0][0] += currentstat;
        tables[0][0] += i & 0x1 ? currentstat : 0;
        tmp[1][i&0x1] += currentstat;
        tables[1][i&0x1] += i & 0x2 ? currentstat : 0;
        tmp[2][i&0x3] += currentstat;
        tables[2][i&0x3] += i & 0x4 ? currentstat : 0;
        tmp[3][i&0x7] += currentstat;
        tables[3][i&0x7] += i & 0x8 ? currentstat : 0;
        tmp[4][i&0xF] += currentstat;
        tables[4][i&0xF] += i & 0x10 ? currentstat : 0;
        tmp[5][i&0x1F] += currentstat;
        tables[5][i&0x1F] += i & 0x20 ? currentstat : 0;
        tmp[6][i&0x3F] += currentstat;
        tables[6][i&0x3F] += i & 0x40 ? currentstat : 0;
        tmp[7][i&0x7F] += currentstat;
        tables[7][i&0x7F] += i & 0x80 ? currentstat : 0;
    }

    for (uint32_t i=0; i<256; i++)
    {
        tables[0][i] = lzaahe_probaGamble( tables[0][i], tmp[0][i] );
    }
}


static void lzaahe_bufferStats( uint32_t* dictionnary, uint8_t* reverse_dictionnary, uint32_t** tables, uint32_t** tmp )
{
    lzaahe_bubbleSort( dictionnary, 256 );

    if (tables != nullptr && tmp != nullptr)
    {
        lzaahe_updateProbaTables( dictionnary, tables, tmp );
    }

    if (reverse_dictionnary != nullptr)
    {
        for (uint32_t i=0; i<256; i++)
            reverse_dictionnary[dictionnary[i] & 0xFF] = i;
    }
}
#endif


static uint32_t lzaahe_potHigher( uint32_t n )
{
    uint32_t low = 0, high = 32;

    for (uint32_t i=0; i<5; i++)
    {
        uint32_t mid = ((high - low) >> 1) + low;
        if (n < (1 << mid)) { high = mid; }
        else { low = mid; }
    }

    return high;
}



#define lzaahe_memcpy8( A, B ) *((uint64_t*) A) = *((const uint64_t*) B)


// Voluntarly overrun the destination buffer by up to 7 bytes
static inline void lzaahe_memcpy_overrun( void* dst, const void* src, uint64_t size )
{
    uint8_t* d = (uint8_t*) dst;
    const uint8_t* s = (const uint8_t*) src;
    uint8_t* const e = ((uint8_t*) dst) + size;

    do { lzaahe_memcpy8(d,s); d+=8; s+=8; } while (d<e);
}

