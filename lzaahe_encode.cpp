/*
** Lib4zip lzaahe encoder implementation.
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
#include <cassert>
#include <time.h>



#include "lzaahe_encode.h"
#include "lzaahe_common.h"



static inline uint32_t matchlen( uint8_t *inbuff, uint32_t first, uint32_t second, uint32_t decoded_size, uint32_t size )
{
    uint32_t maxmatchstrlen = 127+4;
    maxmatchstrlen = (first+maxmatchstrlen < decoded_size) ? maxmatchstrlen : decoded_size-first;
    maxmatchstrlen = (second+maxmatchstrlen) < size ? maxmatchstrlen : size - second;
    maxmatchstrlen = (second-first) < maxmatchstrlen ? second-first : maxmatchstrlen;

    if (maxmatchstrlen >= 4)
    {
        uint32_t i = 4;
        uint8_t *strfirst = inbuff+first;
        uint8_t *strsecond = inbuff+second;

        while ((i != maxmatchstrlen) && (strfirst[i] == strsecond[i])) i++;

        return i;
    }
    else
        return 1;
}


static void init(struct LZAAHECompressionContext* ctx)
{
    memset( ctx->refhashcount, 0, LZAAHE_REFHASH_SZ*sizeof(uint8_t) );

    ctx->id = 0;
    ctx->id_bits = 1;
    ctx->id_mask = 1;
}


static inline uint32_t getHash( uint32_t h )
{
    return (((h & (0xFFFFFFFF - (LZAAHE_REFHASH_SZ - 1))) >> (32-LZAAHE_REFHASH_BITS)) ^ (h & (LZAAHE_REFHASH_SZ - 1)));
}


static inline bool addHit( struct LZAAHECompressionContext *context, uint8_t *input, uint32_t i, uint32_t size, uint32_t &hitlength, uint32_t &hitid, uint32_t &hitpos)
{
    hitid = -1;

    if (i < size-3)
    {
        uint32_t str4 = *((uint32_t*) (input+i));
        uint32_t hash = getHash(str4);
        uint32_t hitidx = hash*LZAAHE_REFHASH_ENTITIES;
        uint32_t j = 0;

        while (j < context->refhashcount[hash] && context->refhash[hitidx+j].sym != str4) j++;

        if (j < context->refhashcount[hash])
        {
            // Hit sym
            uint32_t matchlength = matchlen( input, context->refhash[hitidx+j].pos, i, i, size );

            if (matchlength >= 4)
            {
                context->refhash[hitidx+j].n_occurences++;

                if (context->refhash[hitidx+j].n_occurences == 2)
                {
                    // new ID
                    hitpos = context->refhash[hitidx+j].pos;

                    context->refhash[hitidx+j].hit_id = context->id++;

                    if (context->id > context->id_mask)
                    {
                        context->id_bits++;
                        context->id_mask = (1 << context->id_bits) - 1;
                    }
                }
                else
                    hitid = context->refhash[hitidx+j].hit_id;

                hitlength = matchlength;

                if (matchlength > context->refhash[hitidx+j].matchlen)
                {
                    context->refhash[hitidx+j].pos = i;
                    context->refhash[hitidx+j].matchlen = matchlength;
                }

                return true;
            }
        }
        else if (j < LZAAHE_REFHASH_ENTITIES)
        {
            // New sym
            context->refhash[hitidx+j].sym = str4;
            context->refhash[hitidx+j].pos = i;
            context->refhash[hitidx+j].hit_id = -1;
            context->refhash[hitidx+j].matchlen = 0;
            context->refhash[hitidx+j].n_occurences = 1;

            context->refhashcount[hash]++;
        }
    }

    return false;
}


static void analyzeResults( struct LZAAHECompressionContext* ctx )
{
    uint32_t minscore = 0;
    uint32_t rep = 0;
    uint32_t total = 0;
    uint32_t n_sym = 0;
    uint32_t hscore = 0;

    for (uint32_t i=0; i<LZAAHE_REFHASH_SZ; i++)
    {
        for (uint32_t j=0; j<ctx->refhashcount[i]; j++)
        {
            uint32_t idx = i*LZAAHE_REFHASH_ENTITIES+j;
            uint32_t score = ctx->refhash[idx].matchlen * ctx->refhash[idx].n_occurences;

            if (score > minscore)
                minscore = score;

            if (ctx->refhash[idx].n_occurences > 10)
                rep++;

            if (score > 1024)
                hscore++;

            if (ctx->refhash[idx].hit_id != -1)
                n_sym++;
        }

        total += ctx->refhashcount[i];
    }

    printf( "occurences %u/%u/%u/%u\n", hscore, rep, n_sym, total );
}


extern "C" void lzaaheEncode( struct LZAAHECompressionContext* ctx, uint8_t *inputBlock, uint8_t *outputBlock, uint32_t *outputSize, uint32_t inputSize )
{
    const uint32_t size = inputSize;

    // First write the uncompressed size
    outputBlock[0] = (size & 0xFF);
    outputBlock[1] = ((size >> 8) & 0xFF);
    outputBlock[2] = ((size >> 16) & 0xFF);

    *outputSize = 3;

    init( ctx );

    uint32_t i = 0;
    uint32_t i_mask = 1;
    uint32_t i_bits = 1;
    uint32_t n_matches = 0;
    uint32_t n_matches4 = 0;
    uint32_t n_matchlen = 0;

    uint32_t uncompressed_i = 0;
    uint32_t hit_i = 0;
    uint64_t hit_buff;
    uint32_t hit_bits = 0;
    uint32_t lz_i = 0;
    uint64_t lz_buff;
    uint32_t lz_bits = 0;

    while (i < size)
    {
        uint32_t j = 0;
        uint32_t hitlen, hitpos, hitid;

        bool hit = addHit( ctx, inputBlock, i, size, hitlen, hitpos, hitid );

        hit_buff = (hit_buff << 1) | hit;
        hit_bits++;

        if (hit_bits == 64)
        {
            ctx->hits[hit_i++] = hit_buff;
            hit_bits = 0;
        }

        if (hit)
        {
            // encode length
            if (hitlen == 4)
            {
                lz_buff <<= 1;
                lz_bits ++;
                n_matches4++;
            }
            else
            {
                lz_buff = (lz_buff << 8) | (128 + hitlen - 5);
                lz_bits += 8;
                n_matches++;
            }

            // repeat symbol?
            if (hitid != -1)
            {
                lz_buff = (lz_buff << 1) | 1;
                lz_bits ++;
                lz_buff = (lz_buff << ctx->id_bits) | hitid;
                lz_bits += ctx->id_bits;
            }
            else // no, we have a new symbol
            {
                lz_buff <<= 1;
                lz_bits ++;
                lz_buff = (lz_buff << i_bits) | hitpos;
                lz_bits += i_bits;
            }

            // flush bit buffer?
            if (lz_bits > 31)
            {
                lz_bits -= 32;
                ctx->lz_matches[lz_i++] = lz_buff >> lz_bits;
            }

            // increment the power of 2 of the current position?
            if ((i != 0) && ((i+hitlen) > i_mask))
            {
                i_bits++;
                i_mask = (1 << i_bits) - 1;
            }

            // go forward in the stream
            i += hitlen;
            n_matchlen += hitlen;
        }
        else
        {
            ctx->uncompressed_syms[uncompressed_i++] = inputBlock[i++];
        }

        while (i > i_mask)
        {
            i_bits++;
            i_mask = (1 << i_bits) - 1;
        }

        assert( uncompressed_i <= LZAAHE_BLOCK_SZ );
    }

    // Flush
    /*
    if (lz_bits > 31)
    {
        lz_bits -= 32;
        ctx->lz_matches[lz_i++] = lz_buff >> lz_bits;
    }
    */

    assert( lz_bits <= 32 );

    if (lz_bits != 0)
    {
        ctx->lz_matches[lz_i++] = lz_buff << (32 - lz_bits);
    }

    if (hit_bits != 0)
    {
        ctx->hits[hit_i++] = hit_buff << (64 - lz_bits);
    }

    // Write output
    outputBlock[3] = (uncompressed_i & 0xFF);
    outputBlock[4] = ((uncompressed_i >> 8) & 0xFF);
    outputBlock[5] = ((uncompressed_i >> 16) & 0xFF);

    *outputSize += 3;

    outputBlock[6] = (hit_i & 0xFF);
    outputBlock[7] = ((hit_i >> 8) & 0xFF);
    outputBlock[8] = ((hit_i >> 16) & 0xFF);

    *outputSize += 3;

    lzaahe_memcpy_overrun( outputBlock+*outputSize, ctx->uncompressed_syms, uncompressed_i );

    *outputSize += uncompressed_i;

    lzaahe_memcpy_overrun( outputBlock+*outputSize, ctx->hits, hit_i*8 );

    *outputSize += hit_i*8;

    lzaahe_memcpy_overrun( outputBlock+*outputSize, ctx->lz_matches, lz_i*4 );

    *outputSize += lz_i*4;

    //printf( "%u -> %u | uncompressed_i %u rle_i %u lz_i %u id %u n_matches4 %u n_matches %u\n", size, *outputSize, uncompressed_i, hit_i*8, lz_i*4, ctx->id, n_matches4, n_matches );

    //analyzeResults(ctx);
}

