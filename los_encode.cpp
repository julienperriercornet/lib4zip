/*
** Lib4zip line of sight (los) encoder implementation.
** Copyright (C) 2024 Julien Perrier-cornet
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <time.h>


#include "los_encode.h"
#include "los_common.h"
#include "los_context.h"


static void los_encodeByteStats1(uint8_t current, uint8_t* dict, struct ArithCtx* arith)
{
    uint32_t sum[8] = { 0 }, sum_1[8] = { 0 };

    for (uint32_t i=0; i<256; i++)
    {
        sum[0] += dict[i];
        if ((i & 1) != 0) sum_1[0] += dict[i];
    }

    for (uint32_t i=current&1; i<256; i+=2)
    {
        sum[1] += dict[i];
        if ((i & 2) != 0) sum_1[1] += dict[i];
    }

    for (uint32_t i=current&3; i<256; i+=4)
    {
        sum[2] += dict[i];
        if ((i & 4) != 0) sum_1[2] += dict[i];
    }

    for (uint32_t i=current&7; i<256; i+=8)
    {
        sum[3] += dict[i];
        if ((i & 8) != 0) sum_1[3] += dict[i];
    }

    for (uint32_t i=current&15; i<256; i+=16)
    {
        sum[4] += dict[i];
        if ((i & 16) != 0) sum_1[4] += dict[i];
    }

    for (uint32_t i=current&31; i<256; i+=32)
    {
        sum[5] += dict[i];
        if ((i & 32) != 0) sum_1[5] += dict[i];
    }

    for (uint32_t i=current&63; i<256; i+=64)
    {
        sum[6] += dict[i];
        if ((i & 64) != 0) sum_1[6] += dict[i];
    }

    for (uint32_t i=current&127; i<256; i+=128)
    {
        sum[7] += dict[i];
        if ((i & 128) != 0) sum_1[7] += dict[i];
    }

    for (uint32_t i=0; i<8; i++)
    {
        arith_encodebit( arith, los_probaGamble( sum_1[i], sum[i] ), (current & (1 << i)) != 0 );
    }
}


static void los_encodeByteStats0(uint8_t current, uint32_t* first, struct ArithCtx* arith)
{
    uint32_t sum[8] = { 0 }, sum_1[8] = { 0 };

    for (uint32_t i=0; i<256; i++)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[0] += stat;
        if ((i & 1) != 0) sum_1[0] += stat;
    }

    for (uint32_t i=current&1; i<256; i+=2)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[1] += stat;
        if ((i & 2) != 0) sum_1[1] += stat;
    }

    for (uint32_t i=current&3; i<256; i+=4)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[2] += stat;
        if ((i & 4) != 0) sum_1[2] += stat;
    }

    for (uint32_t i=current&7; i<256; i+=8)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[3] += stat;
        if ((i & 8) != 0) sum_1[3] += stat;
    }

    for (uint32_t i=current&15; i<256; i+=16)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[4] += stat;
        if ((i & 16) != 0) sum_1[4] += stat;
    }

    for (uint32_t i=current&31; i<256; i+=32)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[5] += stat;
        if ((i & 32) != 0) sum_1[5] += stat;
    }

    for (uint32_t i=current&63; i<256; i+=64)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[6] += stat;
        if ((i & 64) != 0) sum_1[6] += stat;
    }

    for (uint32_t i=current&127; i<256; i+=128)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[7] += stat;
        if ((i & 128) != 0) sum_1[7] += stat;
    }

    for (uint32_t i=0; i<8; i++)
    {
        arith_encodebit( arith, los_probaGamble( sum_1[i], sum[i] ), (current & (1 << i)) != 0 );
    }
}


static void los_encodeByteNHit(uint8_t current, uint32_t* first, struct ArithCtx* arith)
{
    uint32_t sum[8] = { 0 }, sum_1[8] = { 0 };

    for (uint32_t i=0; i<256; i++)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[0] += stat;
        if ((i & 1) != 0) sum_1[0] += stat;
    }

    for (uint32_t i=current&1; i<256; i+=2)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[1] += stat;
        if ((i & 2) != 0) sum_1[1] += stat;
    }

    for (uint32_t i=current&3; i<256; i+=4)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[2] += stat;
        if ((i & 4) != 0) sum_1[2] += stat;
    }

    for (uint32_t i=current&7; i<256; i+=8)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[3] += stat;
        if ((i & 8) != 0) sum_1[3] += stat;
    }

    for (uint32_t i=current&15; i<256; i+=16)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[4] += stat;
        if ((i & 16) != 0) sum_1[4] += stat;
    }

    for (uint32_t i=current&31; i<256; i+=32)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[5] += stat;
        if ((i & 32) != 0) sum_1[5] += stat;
    }

    for (uint32_t i=current&63; i<256; i+=64)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[6] += stat;
        if ((i & 64) != 0) sum_1[6] += stat;
    }

    for (uint32_t i=current&127; i<256; i+=128)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[7] += stat;
        if ((i & 128) != 0) sum_1[7] += stat;
    }

    for (uint32_t i=0; i<8; i++)
    {
        arith_encodebit( arith, los_probaGamble( sum_1[i], sum[i] ), (current & (1 << i)) != 0 );
    }
}


extern "C" void losEncode( struct LOSCompressionContext* ctx, uint8_t *inputBlock, uint8_t *outputBlock, uint32_t *outputSize, uint32_t inputSize )
{
    const uint32_t size = inputSize;

    // First write the uncompressed size
    outputBlock[0] = (size & 0xFF);
    outputBlock[1] = ((size >> 8) & 0xFF);
    outputBlock[2] = ((size >> 16) & 0xFF);
    outputBlock[3] = ((size >> 24) & 0xFF);

    *outputSize = 4;

    init( ctx );

    arith_init( ctx->arith, outputBlock+4, inputSize+(inputSize/8)-4 );

    uint32_t i = 0;
    uint32_t accum = 0;
    uint32_t bhit0 = LOS_BHIT_RENORM_TRSH/2, bhit1 = 1; // Avoid divide by 0 when encoding the first byte
    // At the start of the stream we get no hits usually so we speculate accordingly

    // Metrics
    uint32_t nRenormalizations = 0;

    while (i < size)
    {
        uint8_t current = inputBlock[i];

        uint32_t dind = accum & 0xFFFFFF;
        uint32_t d = ctx->dictidx[dind];
        uint32_t bit = 1 << (current & 0x1F);
        uint32_t ind = current >> 5;

        uint32_t *first = ctx->presence + (dind * 16);
        uint32_t *second = ctx->presence + (dind * 16 + 8);

        // Encode current input byte according to stats

        // Encode header bit: hit or not?
        uint32_t bhit;

        if (d != 0xFFFFFFFF)
        {
            bhit = ctx->dict[d+current] != 0;
        }
        else
        {
            bhit = (first[ind] & bit) != 0;
        }

        arith_encodebit(ctx->arith, los_probaGamble( bhit1, bhit0+bhit1 ), bhit);

        // Update bhit stats (cf. *always* update stats *after* encoding the data, based on previous statistics, otherwise the stream can't be decompressed)
        if (bhit) bhit1++;
        else bhit0++;

        // Renormalization of bhit stats. This creates a sliding "window" of statistics for this specific bit
        if (bhit1 == LOS_BHIT_RENORM_TRSH || bhit0 == LOS_BHIT_RENORM_TRSH)
        {
            if (bhit1 > 1) bhit1 >>= 1;
            if (bhit0 > 1) bhit0 >>= 1;
        }

        // Encode byte
        if (bhit && d != 0xFFFFFFFF)
        {
            los_encodeByteStats1(current, ctx->dict+d, ctx->arith);
        }
        else if (bhit && d == 0xFFFFFFFF)
        {
            los_encodeByteStats0(current, first, ctx->arith);
        }
        else
        {
            los_encodeByteNHit(current, first, ctx->arith);
        }

        // Update stats
        uint32_t atleasttwo = first[ind] & bit;

        second[ind] |= atleasttwo;
        first[ind] |= bit;

        if (d != 0xFFFFFFFF)
        {
            if (ctx->dict[d+current] == 255)
            {
                // Renormalize byte stats
                for (uint32_t i=0; i<256; i++)
                {
                    /*
                    if (ctx->dict[d+i] == 1)
                    {
                        // This hit is going to be erased so we reset the bit field as well
                        second[ind] &= ~bit;
                        first[ind] &= ~bit;
                    }
                    */

                    if (ctx->dict[d+i] > 1)
                        ctx->dict[d+i] >>= 1;
                }

                nRenormalizations++;
            }

            ctx->dict[d+current]++;
        }
        else if (atleasttwo != 0)
        {
            if (d == 0xFFFFFFFF && ctx->dictIdx < ctx->dictSz)
            {
                // Create a new byte stat array (yes, this is how you allocate memory in the fastest way possible)
                ctx->dictidx[dind] = ctx->dictIdx;
                ctx->dictIdx += 256;

                // Initialize the byte stat array with current bit statistics (1 for every bit in first set to 1 and otherwise 0, and set current to 2)
                d = ctx->dictidx[dind];

                for (uint32_t i=0; i<256; i++)
                {
                    ctx->dict[d+i] = (first[i>>5] & (1 << (i & 0x1F))) != 0 ? 1 : 0;
                }

                ctx->dict[d+current] = 2;

                if (ctx->dictIdx == ctx->dictSz && ctx->dictSz < (1 << 31))
                {
                    uint8_t* olddict = ctx->dict;
                    ctx->dict = (uint8_t*) align_alloc(MAX_CACHE_LINE_SIZE, ctx->dictSz*2);

                    if (ctx->dict)
                    {
                        aligned_memcpy(ctx->dict, olddict, ctx->dictSz);
                        align_free(olddict);
                        ctx->dictSz *= 2;
                    }
                }
            }
        }

        accum = (accum << 8) | current;
        i++;
    }

    //printf( "ctxt mem: %uM\n", ctx->dictIdx>>20 );

    arith_finalize(ctx->arith);

    *outputSize += arith_getoutptr(ctx->arith);
}

