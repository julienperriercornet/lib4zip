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


#if _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif


static __m256i constant_1 = _mm256_set1_epi32( 1 );



static void init( struct LOSCompressionContext* ctx )
{
    memset( ctx->presence, 0, 128*(1<<24) );
    memset( ctx->dictidx, -1, sizeof(uint32_t)*(1<<24) );
    ctx->dictIdx = 0;
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

    arith_init( ctx->arith, outputBlock+4, (*outputSize)+4 );

    uint32_t i = 0;
    uint32_t accum = 0;
    uint32_t bhit0 = 128, bhit1 = 0; // Avoid divide by 0 when encoding the first byte
    // At the start of the stream we get no hits usually so we speculate accordingly

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

        // Renormalization of bhit stats
        if (bhit1 == 256 || bhit0 == 256)
        {
            bhit1 >>= 1;
            bhit0 >>= 1;
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
                    if (ctx->dict[d+i] == 1)
                    {
                        // This hit is going to be erased so we reset the bit fields as well (necessary? for correctness yes but not 100% sure)
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

    *outputSize += arith_getoutptr(ctx->arith);
}

