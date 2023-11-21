
#include <cstdio>
#include <cstdlib>
#include <cstring>


#include "../lzaahe_context.h"

#define LZAAHE_SORT_STATS
#define LZAAHE_SORT_DEBUG
#include "../lzaahe_common.h"



int test_lzaahe_sort()
{
    int status = 0;

    uint32_t *testarray = (uint32_t*) aligned_alloc( 256, 8*sizeof(uint32_t) );

    if (testarray != nullptr)
    {
        testarray[0] = 0x101;
        testarray[1] = 0x1B5;
        testarray[2] = 0x175;
        testarray[3] = 0x115;
        testarray[4] = 0x145;
        testarray[5] = 0x125;
        testarray[6] = 0x100;
        testarray[7] = 0x14F;

        lzaahe_bubbleSort( testarray, 8 );

        if (testarray[0] != 0x1B5) status |= 1;
        if (testarray[1] != 0x175) status |= 1;
        if (testarray[2] != 0x14F) status |= 1;
        if (testarray[3] != 0x145) status |= 1;
        if (testarray[4] != 0x125) status |= 1;
        if (testarray[5] != 0x115) status |= 1;
        if (testarray[6] != 0x101) status |= 1;
        if (testarray[7] != 0x100) status |= 1;

        testarray[3] -= 0x100;
        lzaahe_bubbleSort( testarray, 8 );

        if (testarray[0] != 0x1B5) status |= 1;
        if (testarray[1] != 0x175) status |= 1;
        if (testarray[2] != 0x14F) status |= 1;
        if (testarray[3] != 0x125) status |= 1;
        if (testarray[4] != 0x115) status |= 1;
        if (testarray[5] != 0x101) status |= 1;
        if (testarray[6] != 0x100) status |= 1;
        if (testarray[7] != 0x45) status |= 1;

        testarray[6] -= 0x100;
        lzaahe_insertionSingleSort( testarray, 6, 8 );

        if (testarray[0] != 0x1B5) status |= 1;
        if (testarray[1] != 0x175) status |= 1;
        if (testarray[2] != 0x14F) status |= 1;
        if (testarray[3] != 0x125) status |= 1;
        if (testarray[4] != 0x115) status |= 1;
        if (testarray[5] != 0x101) status |= 1;
        if (testarray[6] != 0x45) status |= 1;
        if (testarray[7] != 0x0) status |= 1;

        free( testarray );
    }

    return status;
}


int test_lzaahe_context()
{
    LZAAHEContext* context = allocateLZAAHEContext( 10 );

    if (context != nullptr)
    {
        deallocateLZAAHEContext(context);
        return 0;
    }
    else
    {
        return 1;
    }
}


int main( int argc, const char** argv )
{
    int status = -1;

    if (argc != 2) return -2;

    if (strcmp(argv[1], "test_lzaahe_context") == 0)
        status = test_lzaahe_context();
    else if (strcmp(argv[1], "test_lzaahe_sort") == 0)
        status = test_lzaahe_sort();

    return status;
}
