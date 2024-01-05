#include <cstdio>
#include <cstdlib>
#include <cstring>


#include "lzaahe_decode.h"
#include "lzaahe_common.h"


extern "C" void lzaaheDecode( struct LZAAHEContext* ctx )
{
    uint32_t size = 0;

    size = ctx->inputBlock[0];
    size |= ctx->inputBlock[1] << 8;
    size |= ctx->inputBlock[2] << 16;

    arith_init(ctx->arithEncoder, ctx->inputBlock+3, ctx->inputSize-3);

    for (uint32_t i=0; i<256; i++)
        ctx->dict[i] = (1 << 8) | i;

    arith_prefetch(ctx->arithEncoder);

    lzaahe_bufferStats( ctx->dict, nullptr, ctx->proba_tables, ctx->tmp_tables );

    for (uint32_t i=0; i<size; i++)
    {
        uint8_t current_input = 0;

        for (uint32_t j=0; j<8; j++)
            current_input |= arith_decodebit(ctx->arithEncoder, ctx->proba_tables[j][current_input]) << j;

        uint8_t sym = ctx->dict[current_input] & 0xFF;

        ctx->outputBlock[i] = sym;
    }

    ctx->outputSize = size;
}

