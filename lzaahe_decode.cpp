#include <cstdio>
#include <cstdlib>
#include <cstring>


#include "lzaahe_decode.h"
#include "lzaahe_common.h"


static void readHeader(uint32_t *dict, struct ArithCtx *arith)
{
    uint32_t encodePot = 0;

    // TODO: header is currently uncompressed
    encodePot = arith_decodebit(arith, 1<<(ARITH_PRECISION-1));
    encodePot |= arith_decodebit(arith, 1<<(ARITH_PRECISION-1)) << 1;
    encodePot |= arith_decodebit(arith, 1<<(ARITH_PRECISION-1)) << 2;

    uint32_t highestPot = encodePot + 15;

    for (uint32_t i=0; i<256; i++)
    {
        uint32_t stat = 0;

        for (uint32_t j=0; j<highestPot; j++)
        {
            stat |= arith_decodebit(arith, 1<<(ARITH_PRECISION-1)) << j;
        }

        dict[i] = (stat << 8) | i;
    }
}


extern "C" void lzaaheDecode( struct LZAAHEContext* ctx )
{
    uint32_t size = 0;

    size = ctx->inputBlock[0];
    size |= ctx->inputBlock[1] << 8;
    size |= ctx->inputBlock[2] << 16;

    arith_init(ctx->arithEncoder, ctx->inputBlock+3, ctx->inputSize-3);

    arith_prefetch(ctx->arithEncoder);

    readHeader( ctx->dict, ctx->arithEncoder );

    lzaahe_bufferStats( ctx->dict, nullptr, ctx->proba_tables, ctx->tmp_tables );

    for (uint32_t i=0; i<size; i++)
    {
        uint8_t current_input = 0;

        for (uint32_t j=0; j<8; j++)
            current_input |= arith_decodebit(ctx->arithEncoder, ctx->proba_tables[j][current_input]) << j;

        uint8_t sym = ctx->dict[current_input] & 0xFF;

        ctx->outputBlock[i] = sym;
    }

}
