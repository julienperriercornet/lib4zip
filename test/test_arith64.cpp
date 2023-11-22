
#include <cstdio>
#include <cstdlib>


#include "../arith64.h"


int test_arith64()
{
    int status = 0;

    uint8_t* buffer = (uint8_t*) malloc( 256 );

    struct ArithCtx compressctx, decompressctx;

    arith_init(&compressctx, buffer, 256);

    arith_encodebit(&compressctx, 1<<(ARITH_PRECISION-1), 1);
    arith_encodebit(&compressctx, 1<<(ARITH_PRECISION-1), 0);
    arith_encodebit(&compressctx, 1<<(ARITH_PRECISION-1), 1);
    arith_encodebit(&compressctx, 1<<(ARITH_PRECISION-1), 1);
    arith_encodebit(&compressctx, 1<<(ARITH_PRECISION-1), 0);
    arith_encodebit(&compressctx, 1<<(ARITH_PRECISION-1), 0);
    arith_encodebit(&compressctx, 1<<(ARITH_PRECISION-1), 1);
    arith_encodebit(&compressctx, 1<<(ARITH_PRECISION-1), 0);

    arith_finalize(&compressctx);

    arith_init(&decompressctx, buffer, 256);

    arith_prefetch(&decompressctx);

    status |= arith_decodebit(&decompressctx, 1<<(ARITH_PRECISION-1)) != 1;
    status |= arith_decodebit(&decompressctx, 1<<(ARITH_PRECISION-1)) != 0;
    status |= arith_decodebit(&decompressctx, 1<<(ARITH_PRECISION-1)) != 1;
    status |= arith_decodebit(&decompressctx, 1<<(ARITH_PRECISION-1)) != 1;
    status |= arith_decodebit(&decompressctx, 1<<(ARITH_PRECISION-1)) != 0;
    status |= arith_decodebit(&decompressctx, 1<<(ARITH_PRECISION-1)) != 0;
    status |= arith_decodebit(&decompressctx, 1<<(ARITH_PRECISION-1)) != 1;
    status |= arith_decodebit(&decompressctx, 1<<(ARITH_PRECISION-1)) != 0;

    free( buffer );

    return status;
}
