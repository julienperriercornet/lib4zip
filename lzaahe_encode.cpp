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
    uint32_t i = 4; // The first 4 bytes of both strings are already matching at this point
    const uint32_t maxmatchstrlen = 136;

    if ((first+maxmatchstrlen) < decoded_size && (second+maxmatchstrlen) < size)
    {
        while (i < maxmatchstrlen && inbuff[first+i] == inbuff[second+i]) i++;
    }
    else
    {
        while ((first+i) < decoded_size && (second+i) < size && i < maxmatchstrlen && inbuff[first+i] == inbuff[second+i]) i++;
    }

    return i < maxmatchstrlen ? i : maxmatchstrlen-1;
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


static inline bool addHit(uint8_t *inbuff, uint32_t size, struct LZAAHECompressionContext::SymRef *refhash, uint8_t *refhashcount, uint32_t hash, uint32_t str4, uint32_t pos, uint32_t &matchlength, uint32_t &hitidx, uint32_t &hitpos)
{
    bool hashHit = false;
    uint32_t i = 0;

    while (i < refhashcount[hash] && refhash[hash*LZAAHE_REFHASH_ENTITIES+i].sym != str4)
    {
        i++;
    }

    if (i < refhashcount[hash])
    {
        // Hit previous sym
        hitidx = hash*LZAAHE_REFHASH_ENTITIES+i;
        hitpos = refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest & (LZAAHE_REFHASH_SZ-1);

        uint32_t maxlen = refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest >> LZAAHE_REFHASH_BITS;

        matchlength = matchlen( inbuff, hitpos, pos, pos, size );

        if (matchlength > maxlen) {
            refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest = hitpos | (matchlength << LZAAHE_REFHASH_BITS);
        }

        return true;
    }
    else if (i >= refhashcount[hash] && i < LZAAHE_REFHASH_ENTITIES)
    {
        refhash[hash*LZAAHE_REFHASH_ENTITIES+i].sym = str4;
        refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest = pos;
        refhash[hash*LZAAHE_REFHASH_ENTITIES+i].hit_id = -1;

        refhashcount[hash]++;
    }

    return false;
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

    FILE *log = fopen( "compression.log", "w" );
    const uint32_t loghitid = 51;

    while (i < size)
    {
        // lz string match search
        uint32_t str4 = 0;
        uint32_t strhash = 0;
        uint32_t hitlen = 1;
        uint32_t hitidx = -1;
        uint32_t hitpos = -1;

        if (i < size-3)
        {
            str4 = *((uint32_t*) (ctx->inputBlock+i));
            strhash = lzaahe_getHash(str4);
        }

        if ((i < size-3) && addHit(ctx->inputBlock, size, ctx->refhash, ctx->refhashcount, strhash, str4, i, hitlen, hitidx, hitpos))
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

            n_matchlen += hitlen;
            n_matches++;
        }
        else
        {
            uint8_t sym = ctx->inputBlock[i];

            bitio_write_bit( ctx->io, 0 );
            writebits( ctx, sym, 8 );

            n_symbols++;
            hitlen = 1;
        }

        if ((i != 0) && ((i+hitlen) > i_mask))
        {
            i_bits++;
            i_mask = (1 << i_bits) - 1;
        }

        i += hitlen;
    }

    bitio_finalize( ctx->io );

    ctx->outputSize += bitio_getoutptr( ctx->io );
}

