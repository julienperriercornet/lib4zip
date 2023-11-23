#include <cstdio>
#include <cstdlib>
#include <cstring>


#include "lzaahe_encode.h"
#include "lzaahe_common.h"


static void makeStats( uint8_t *buffer, uint32_t size, uint32_t *stats )
{
    memset( stats, 0, 256*sizeof(uint32_t) );

    for (uint32_t i=0; i<size; i++)
        stats[buffer[i]]++;
}


static uint32_t potHigher( uint32_t n )
{
    uint32_t low = 0, high = 32;

    while ((high - low) != 0)
    {
        if (n < (1<<((high - low) >> 1))) { high = (high - low) >> 1; }
        else { low = (high - low) >> 1; }
    }

    return high;
}


static void writeHeader( uint32_t *stats, struct ArithCtx *arith )
{
    uint32_t highestPot = potHigher( stats[0] );

    for (uint32_t i=1; i<256; i++)
    {
        unsigned int pot = potHigher( stats[i] );
        if (pot > highestPot) highestPot = pot;
    }

    uint32_t encodePot = highestPot - 15;

    // TODO: header is currently uncompressed
    arith_encodebit(arith, 1<<(ARITH_PRECISION-1), encodePot & 1);
    arith_encodebit(arith, 1<<(ARITH_PRECISION-1), (encodePot >> 1) & 1);
    arith_encodebit(arith, 1<<(ARITH_PRECISION-1), (encodePot >> 2) & 1);

    for (uint32_t i=0; i<256; i++)
    {
        for (uint32_t j=0; j<highestPot; j++)
        {
            arith_encodebit(arith, 1<<(ARITH_PRECISION-1), (stats[i] >> j) & 1);
        }
    }
}


extern "C" void lzaaheEncode( struct LZAAHEContext* ctx )
{
    uint32_t size = ctx->inputSize;

    // First write the uncompressed size
    ctx->outputBlock[0] = (size & 0xFF);
    ctx->outputBlock[1] = ((size >> 8) & 0xFF);
    ctx->outputBlock[2] = ((size >> 16) & 0xFF);

    ctx->outputSize = 3;

    arith_init( ctx->arithEncoder, ctx->outputBlock+3, LZAAHE_OUTPUT_SZ-3 );

    makeStats( ctx->inputBlock, size, ctx->stats );

    writeHeader( ctx->stats, ctx->arithEncoder );

    for (uint32_t i=0; i<256; i++)
        ctx->dict[i] = (ctx->stats[i] << 8) | i;

    lzaahe_bufferStats( ctx->dict, ctx->reverse_dictionnary, ctx->proba_tables, ctx->tmp_tables );

    for (uint32_t i=0; i<size; i++)
    {
        uint8_t sym = ctx->inputBlock[i];
        uint8_t current = ctx->reverse_dictionnary[sym];

        for (uint32_t j=0; j<8; j++)
        {
            arith_encodebit( ctx->arithEncoder, ctx->proba_tables[j][j==0?0:current&(1<<(j-1))], (current&(1<<j))>>j);
        }
    }

    arith_finalize( ctx->arithEncoder );
    ctx->outputSize += arith_getoutptr( ctx->arithEncoder );
}
