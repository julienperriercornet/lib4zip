#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>



#include "lzaahe_decode.h"
#include "lzaahe_common.h"



static inline uint32_t readbits( struct LZAAHEDecompressionContext* ctx, uint32_t bits )
{
    return bitio_read( ctx->io, bits );
}


extern "C" void lzaaheDecode( struct LZAAHEDecompressionContext* ctx )
{
    uint32_t size = 0;

    size = ctx->inputBlock[0];
    size |= ctx->inputBlock[1] << 8;
    size |= ctx->inputBlock[2] << 16;

    bitio_init( ctx->io, ctx->inputBlock+3, ctx->inputSize-3 );

    bitio_prefetch( ctx->io );

    uint32_t i = 0;
    uint32_t i_mask = 1;
    uint32_t i_bits = 1;
    uint32_t id_cnt = 0;
    uint32_t id_bits = 1;
    uint32_t id_mask = 1;

    while (i < size)
    {
        uint32_t hitlen = 1;
        uint32_t hitid = -1;
        uint32_t hitpos = -1;

        if (bitio_read_bit( ctx->io ))
        {
            bool is_len4 = bitio_read_bit( ctx->io );

            if (is_len4)
            {
                hitlen = 4;
            }
            else
            {
                hitlen = readbits( ctx, 2 ) + 5;

                if (hitlen == 8)
                    hitlen = readbits( ctx, 7 ) + 8;
            }

            bool is_newhit = bitio_read_bit( ctx->io );

            if (is_newhit)
            {
                hitpos = readbits( ctx, i_bits );

                lzaahe_memcpy_overrun( ctx->outputBlock+i, ctx->outputBlock+hitpos, hitlen );

                ctx->symlist[id_cnt].pos = hitpos;
                ctx->symlist[id_cnt].len = hitlen;

                id_cnt++;

                if (id_cnt != 0 && (id_cnt & id_mask) == 0)
                {
                    id_bits++;
                    id_mask = (1 << id_bits) - 1;
                }
            }
            else
            {
                hitid = readbits( ctx, id_bits );
                hitpos = ctx->symlist[hitid].pos;

                lzaahe_memcpy_overrun( ctx->outputBlock+i, ctx->outputBlock+hitpos, hitlen );

                if (hitlen > ctx->symlist[hitid].len)
                {
                    ctx->symlist[hitid].pos = hitpos;
                    ctx->symlist[hitid].len = hitlen;
                }
            }
        }
        else
        {
            ctx->outputBlock[i] = readbits( ctx, 8 );
        }

        if ((i != 0) && ((i+hitlen) > i_mask))
        {
            i_bits++;
            i_mask = (1 << i_bits) - 1;
        }

        i += hitlen;
    }

    ctx->outputSize = size;
}

