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
