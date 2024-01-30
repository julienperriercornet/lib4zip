#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <time.h>
#include <vector>
#include <algorithm>



#include "lzaahe_encode.h"
#include "lzaahe_common.h"



static inline uint32_t matchlen( uint8_t *inbuff, uint32_t first, uint32_t second, uint32_t decoded_size, uint32_t size )
{
    uint32_t maxmatchstrlen = (first+maxmatchstrlen < decoded_size) ? 136 : decoded_size-first;
    maxmatchstrlen = (second+maxmatchstrlen) < size ? maxmatchstrlen : size - second;
    maxmatchstrlen = (second-first) < maxmatchstrlen ? second-first : maxmatchstrlen;

    if (maxmatchstrlen >= 4)
    {
        uint32_t i = 0;
        uint8_t *strfirst = inbuff+first+4;
        uint8_t *strstart = inbuff+first;
        uint8_t *strsecond = inbuff+second+4;
        uint8_t *strend = inbuff+first+maxmatchstrlen;

        while ((strfirst+i != strend) && (strfirst[i] == strsecond[i])) i++;

        return (uint32_t) (strfirst-strstart);
    }
    else
        return 1;
}


static inline void writebits( struct LZAAHECompressionContext* ctx, uint32_t d, uint32_t bits )
{
    bitio_write( ctx->io, bits, d);
}


static void init(struct LZAAHECompressionContext* ctx)
{
    memset( ctx->refhashcount, 0, LZAAHE_REFHASH_SZ*sizeof(uint8_t) );

    ctx->refcount.id_cnt = 0;
    ctx->refcount.id_bits = 1;
    ctx->refcount.id_mask = 1;
}


static inline uint32_t getHash( uint32_t h )
{
    return (((h & (0xFFFFFFFF - (LZAAHE_REFHASH_SZ - 1))) >> (32-LZAAHE_REFHASH_BITS)) ^ (h & (LZAAHE_REFHASH_SZ - 1)));
}


static inline bool addHit(uint8_t *inbuff, uint32_t size, struct LZAAHECompressionContext::SymRef *refhash, uint8_t *refhashcount, uint32_t hash, uint32_t str4, uint32_t pos, uint32_t &matchlength, uint32_t &hitidx, uint32_t &hitpos)
{
    uint32_t i = 0;

    while (i < refhashcount[hash] && refhash[hash*LZAAHE_REFHASH_ENTITIES+i].sym != str4) i++;

    if (i < refhashcount[hash])
    {
        // Hit sym
        hitidx = hash*LZAAHE_REFHASH_ENTITIES+i;
        hitpos = refhash[hitidx].longest;

        refhash[hitidx].n_occurences++;

        matchlength = matchlen( inbuff, hitpos, pos, pos, size );

        if (matchlength >= 4)
        {
            if (matchlength > refhash[hitidx].matchlen)
            {
                refhash[hitidx].longest = hitpos;
                refhash[hitidx].matchlen = matchlength;
            }

            return true;
        }
    }
    else if (i >= refhashcount[hash] && i < LZAAHE_REFHASH_ENTITIES)
    {
        // New sym
        hitidx = hash*LZAAHE_REFHASH_ENTITIES+i;

        refhash[hitidx].sym = str4;
        refhash[hitidx].longest = pos;
        refhash[hitidx].hit_id = -1;
        refhash[hitidx].matchlen = 0;
        refhash[hitidx].n_occurences = 1;

        refhashcount[hash]++;
    }

    matchlength = 1;

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


extern "C" void lzaaheEncode( struct LZAAHECompressionContext* ctx )
{
    uint32_t size = ctx->inputSize;

    // First write the uncompressed size
    ctx->outputBlock[0] = (size & 0xFF);
    ctx->outputBlock[1] = ((size >> 8) & 0xFF);
    ctx->outputBlock[2] = ((size >> 16) & 0xFF);

    ctx->outputSize = 3;

    bitio_init( ctx->io, ctx->outputBlock+3, LZAAHE_OUTPUT_SZ-3 );
    init( ctx );

    uint32_t i = 0;
    uint32_t i_mask = 1;
    uint32_t i_bits = 1;
    uint32_t n_matches = 0;
    uint32_t n_symbols = 0;
    uint32_t n_matchlen = 0;

    while (i < size)
    {
        // lz string match search
        uint32_t hitlen = 1;
        uint32_t hitidx = -1;
        uint32_t hitpos = -1;

        bool addhit = false;

        if (i < size-3)
        {
            uint32_t str4 = *((uint32_t*) (ctx->inputBlock+i));

            addhit = addHit(ctx->inputBlock, size, ctx->refhash, ctx->refhashcount, getHash(str4), str4, i, hitlen, hitidx, hitpos);
        }

        if (addhit /*&& hitidx != -1*/ && !((ctx->refhash[hitidx].hit_id == -1) && (ctx->refcount.id_cnt >= LZAAHE_MAX_SYMBOLS)))
        {
            bitio_write_bit( ctx->io, 1 );
            bitio_write_bit( ctx->io, hitlen == 4 );

            if (hitlen != 4)
            {
                if (hitlen < 8)
                    writebits( ctx, hitlen-5, 2 );
                else
                {
                    writebits( ctx, 3, 2 );
                    writebits( ctx, hitlen-8, 7 );
                }
            }

            if (ctx->refhash[hitidx].hit_id == -1)
            {
                ctx->refhash[hitidx].hit_id = ctx->refcount.id_cnt++;
                bitio_write_bit( ctx->io, 1 );
                writebits( ctx, hitpos, i_bits );

                if (ctx->refcount.id_cnt != 0 && (ctx->refcount.id_cnt & ctx->refcount.id_mask) == 0)
                {
                    ctx->refcount.id_bits++;
                    ctx->refcount.id_mask = (1 << ctx->refcount.id_bits) - 1;
                }
            }
            else
            {
                bitio_write_bit( ctx->io, 0 );
                writebits( ctx, ctx->refhash[hitidx].hit_id, ctx->refcount.id_bits );
            }
        }
        else
        {
            uint8_t sym = ctx->inputBlock[i];

            bitio_write_bit( ctx->io, 0 );
            writebits( ctx, sym, 8 );
        }

        if ((i != 0) && ((i+hitlen) > i_mask))
        {
            i_bits++;
            i_mask = (1 << i_bits) - 1;
        }

        i += hitlen;
    }

    bitio_finalize( ctx->io );

    //analyzeResults(ctx);

    ctx->outputSize += bitio_getoutptr( ctx->io );
}

