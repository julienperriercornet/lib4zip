/*
** Lib4zip lzaahe decoder implementation.
** Copyright (C) 2022-2024 Julien Perrier-cornet
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
#include <vector>



#include "los_decode.h"
#include "los_common.h"


static uint8_t los_decodeByteStats1(uint8_t* dict, struct ArithCtx* arith)
{
    uint8_t current = 0;
    uint32_t sum[8] = { 0 }, sum_1[8] = { 0 };

    for (uint32_t i=0; i<256; i++)
    {
        sum[0] += dict[i];
        if ((i & 1) != 0) sum_1[0] += dict[i];
    }

    current = arith_decodebit(arith, los_probaGamble( sum_1[0], sum[0] ));

    for (uint32_t i=current; i<256; i+=2)
    {
        sum[1] += dict[i];
        if ((i & 2) != 0) sum_1[1] += dict[i];
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[1], sum[1] )) << 1;

    for (uint32_t i=current; i<256; i+=4)
    {
        sum[2] += dict[i];
        if ((i & 4) != 0) sum_1[2] += dict[i];
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[2], sum[2] )) << 2;

    for (uint32_t i=current; i<256; i+=8)
    {
        sum[3] += dict[i];
        if ((i & 8) != 0) sum_1[3] += dict[i];
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[3], sum[3] )) << 3;

    for (uint32_t i=current; i<256; i+=16)
    {
        sum[4] += dict[i];
        if ((i & 16) != 0) sum_1[4] += dict[i];
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[4], sum[4] )) << 4;

    for (uint32_t i=current; i<256; i+=32)
    {
        sum[5] += dict[i];
        if ((i & 32) != 0) sum_1[5] += dict[i];
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[5], sum[5] )) << 5;

    for (uint32_t i=current; i<256; i+=64)
    {
        sum[6] += dict[i];
        if ((i & 64) != 0) sum_1[6] += dict[i];
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[6], sum[6] )) << 6;

    for (uint32_t i=current; i<256; i+=128)
    {
        sum[7] += dict[i];
        if ((i & 128) != 0) sum_1[7] += dict[i];
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[7], sum[7] )) << 7;

    return current;
}


static uint8_t los_decodeByteStats0(uint32_t* first, struct ArithCtx* arith)
{
    uint8_t current = 0;
    uint32_t sum[8] = { 0 }, sum_1[8] = { 0 };

    for (uint32_t i=0; i<256; i++)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[0] += stat;
        if ((i & 1) != 0) sum_1[0] += stat;
    }

    current = arith_decodebit(arith, los_probaGamble( sum_1[0], sum[0] ));

    for (uint32_t i=current; i<256; i+=2)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[1] += stat;
        if ((i & 2) != 0) sum_1[1] += stat;
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[1], sum[1] )) << 1;

    for (uint32_t i=current; i<256; i+=4)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[2] += stat;
        if ((i & 4) != 0) sum_1[2] += stat;
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[2], sum[2] )) << 2;

    for (uint32_t i=current; i<256; i+=8)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[3] += stat;
        if ((i & 8) != 0) sum_1[3] += stat;
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[3], sum[3] )) << 3;

    for (uint32_t i=current; i<256; i+=16)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[4] += stat;
        if ((i & 16) != 0) sum_1[4] += stat;
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[4], sum[4] )) << 4;

    for (uint32_t i=current; i<256; i+=32)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[5] += stat;
        if ((i & 32) != 0) sum_1[5] += stat;
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[5], sum[5] )) << 5;

    for (uint32_t i=current; i<256; i+=64)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[6] += stat;
        if ((i & 64) != 0) sum_1[6] += stat;
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[6], sum[6] )) << 6;

    for (uint32_t i=current; i<256; i+=128)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) != 0 ? 1 : 0;
        sum[7] += stat;
        if ((i & 128) != 0) sum_1[7] += stat;
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[7], sum[7] )) << 7;

    return current;
}


static uint8_t los_decodeByteNHit(uint32_t* first, struct ArithCtx* arith)
{
    uint8_t current = 0;
    uint32_t sum[8] = { 0 }, sum_1[8] = { 0 };

    for (uint32_t i=0; i<256; i++)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[0] += stat;
        if ((i & 1) != 0) sum_1[0] += stat;
    }

    current = arith_decodebit(arith, los_probaGamble( sum_1[0], sum[0] ));

    for (uint32_t i=current; i<256; i+=2)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[1] += stat;
        if ((i & 2) != 0) sum_1[1] += stat;
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[1], sum[1] )) << 1;

    for (uint32_t i=current; i<256; i+=4)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[2] += stat;
        if ((i & 4) != 0) sum_1[2] += stat;
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[2], sum[2] )) << 2;

    for (uint32_t i=current; i<256; i+=8)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[3] += stat;
        if ((i & 8) != 0) sum_1[3] += stat;
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[3], sum[3] )) << 3;

    for (uint32_t i=current; i<256; i+=16)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[4] += stat;
        if ((i & 16) != 0) sum_1[4] += stat;
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[4], sum[4] )) << 4;

    for (uint32_t i=current; i<256; i+=32)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[5] += stat;
        if ((i & 32) != 0) sum_1[5] += stat;
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[5], sum[5] )) << 5;

    for (uint32_t i=current; i<256; i+=64)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[6] += stat;
        if ((i & 64) != 0) sum_1[6] += stat;
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[6], sum[6] )) << 6;

    for (uint32_t i=current; i<256; i+=128)
    {
        uint32_t bit = 1 << (i & 0x1F);
        uint32_t ind = i >> 5;
        uint32_t stat = (first[ind] & bit) == 0 ? 1 : 0;
        sum[7] += stat;
        if ((i & 128) != 0) sum_1[7] += stat;
    }

    current |= arith_decodebit(arith, los_probaGamble( sum_1[7], sum[7] )) << 7;

    return current;
}



extern "C" void losDecode( struct LOSCompressionContext* ctx, uint8_t *inputBlock, uint8_t *outputBlock, uint32_t *outputSize, uint32_t inputSize )
{
    uint32_t size = 0;

    size = inputBlock[0];
    size |= inputBlock[1] << 8;
    size |= inputBlock[2] << 16;
    size |= inputBlock[3] << 24;

    init( ctx );

    arith_init(ctx->arith, inputBlock+4, inputSize-4);

    arith_prefetch(ctx->arith);

    uint32_t i = 0;
    uint32_t accum = 0;
    uint32_t bhit0 = 256, bhit1 = 1;

    while (i < size)
    {
        uint32_t dind = accum & 0xFFFFFF;
        uint32_t d = ctx->dictidx[dind];

        uint32_t *first = ctx->presence + (dind * 16);
        uint32_t *second = ctx->presence + (dind * 16 + 8);

        // Decode bhit
        uint32_t bhit = arith_decodebit(ctx->arith, los_probaGamble( bhit1, bhit0+bhit1 ));

        if (bhit) bhit1++;
        else bhit0++;

        // Renormalization
        if (bhit1 == 65536 || bhit0 == 65536)
        {
            bhit1 = (bhit1 >> 1) | 1;
            bhit0 = (bhit0 >> 1) | 1;
        }

        // Decode byte
        uint8_t current = 0;

        if (bhit && d != 0xFFFFFFFF)
        {
            current = los_decodeByteStats1(ctx->dict+d, ctx->arith);
        }
        else if (bhit && d == 0xFFFFFFFF)
        {
            current = los_decodeByteStats0(first, ctx->arith);
        }
        else
        {
            current = los_decodeByteNHit(first, ctx->arith);
        }

        outputBlock[i] = current;

        uint32_t bit = 1 << (current & 0x1F);
        uint32_t ind = current >> 5;

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
                    if (ctx->dict[d+i] == 1)
                    {
                        // This hit is going to be erased so we reset the bit field as well
                        second[ind] &= ~bit;
                        first[ind] &= ~bit;
                    }

                    ctx->dict[d+i] >>= 1;
                }
            }

            ctx->dict[d+current]++;
        }
        else if (atleasttwo != 0)
        {
            if (d == 0xFFFFFFFF)
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
            }
        }

        accum = (accum << 8) | current;
        i++;
    }

    *outputSize = size;
}

