#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>


#include "lzaahe_encode.h"
#include "lzaahe_common.h"


extern "C" void lzaaheEncode( struct LZAAHEContext* ctx )
{
    uint32_t size = ctx->inputSize;

    // First write the uncompressed size
    ctx->outputBlock[0] = (size & 0xFF);
    ctx->outputBlock[1] = ((size >> 8) & 0xFF);
    ctx->outputBlock[2] = ((size >> 16) & 0xFF);

    ctx->outputSize = 3;

    arith_init( ctx->arithEncoder, ctx->outputBlock+3, LZAAHE_OUTPUT_SZ-3 );

    for (uint32_t i=0; i<256; i++)
        ctx->dict[i] = (1 << 8) | i;

    lzaahe_bufferStats( ctx->dict, ctx->reverse_dictionnary, ctx->proba_tables, ctx->tmp_tables );

    for (uint32_t i=0; i<size; i++)
    {
        uint8_t sym = ctx->inputBlock[i];
        uint8_t current = ctx->reverse_dictionnary[sym];

        for (uint32_t j=0; j<8; j++)
        {
            arith_encodebit( ctx->arithEncoder, ctx->proba_tables[j][current&((1<<j)-1)], (current&(1<<j))>>j);
        }
    }

    arith_finalize( ctx->arithEncoder );

    ctx->outputSize += arith_getoutptr( ctx->arithEncoder );
}
