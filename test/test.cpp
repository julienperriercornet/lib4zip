
#include <cstdio>
#include <cstdlib>
#include <cstring>


#include "../lzaahe_context.h"
#define LZAAHE_SORT_STATS
#define LZAAHE_SORT_DEBUG
#include "../lzaahe_common.h"
#include "../bitio.h"


extern int test_arith32();
extern int test_arith64();


int test_lzaahe_sort()
{
    int status = 0;

    uint32_t *testarray = (uint32_t*) align_alloc( 256, 8*sizeof(uint32_t) );

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

        align_free( testarray );
    }

    return status;
}


int test_lzaahe_context()
{
    LZAAHECompressionContext* context = lzaaheAllocateCompression();

    if (context != nullptr)
    {
        lzaaheDeallocateCompression(context);
        return 0;
    }
    else
    {
        return 1;
    }
}


int test_lzaahe_bitio()
{
    int status = 0;

    uint8_t *buffer = (uint8_t*) align_alloc( 256, 256*sizeof(uint8_t) );

    struct BitIOCtx writectx;

    bitio_init( &writectx, buffer, 256 );

    bitio_write_bit( &writectx, 0 );
    bitio_write_bit( &writectx, 1 );
    bitio_write( &writectx, 6, 42 );
    bitio_write( &writectx, 11, 1337 );
    bitio_write( &writectx, 4, 13 );
    bitio_write( &writectx, 15, 31337 );
    bitio_write_bit( &writectx, 0 );

    bitio_finalize( &writectx );

    struct BitIOCtx readctx;

    bitio_init( &readctx, buffer, 256 );

    bitio_prefetch( &readctx );

    if (bitio_read_bit( &readctx ) != 0) status |= 1;
    if (bitio_read_bit( &readctx ) != 1) status |= 1;
    if (bitio_read( &readctx, 6 ) != 42) status |= 1;
    if (bitio_read( &readctx, 11 ) != 1337) status |= 1;
    if (bitio_read( &readctx, 4 ) != 13) status |= 1;
    if (bitio_read( &readctx, 15 ) != 31337) status |= 1;
    if (bitio_read_bit( &readctx ) != 0) status |= 1;

    align_free( buffer );

    return status;
}


int main( int argc, const char** argv )
{
    int status = -1;

    if (argc != 2) return -2;

    if (strcmp(argv[1], "test_lzaahe_context") == 0)
        status = test_lzaahe_context();
    else if (strcmp(argv[1], "test_lzaahe_sort") == 0)
        status = test_lzaahe_sort();
    else if (strcmp(argv[1], "test_arith32") == 0)
        status = test_arith32();
    else if (strcmp(argv[1], "test_arith64") == 0)
        status = test_arith64();
    else if (strcmp(argv[1], "test_bitio") == 0)
        status = test_lzaahe_bitio();

    return status;
}
