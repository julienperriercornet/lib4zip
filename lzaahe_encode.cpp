#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>



#include "lzaahe_encode.h"
#include "lzaahe_common.h"



static inline uint32_t matchlen( uint8_t *inbuff, uint32_t first, uint32_t second, uint32_t decoded_size, uint32_t size )
{
    uint32_t i = 4; // The first 4 bytes of both strings are already matching at this point

    while ((first+i) < decoded_size && (second+i) < size && i < 68 && inbuff[first+i] == inbuff[second+i]) i++;

    return i;
}


static inline bool addHit(uint8_t *inbuff, uint32_t size, SymRef *refhash, uint8_t *refhashcount, uint32_t hash, uint32_t str4, uint32_t pos, uint32_t &matchlength)
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
        // The top bits are a hit counter, the lower bits are the longest match pos in the uncompressed data array
        refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest1 += LZAAHE_REFHASH_SZ;

        uint32_t len1 = matchlen( inbuff, pos, refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest1 & (LZAAHE_BLOCK_SZ-1), pos, size );
        uint32_t len2 = matchlen( inbuff, pos, refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest2 & (LZAAHE_BLOCK_SZ-1), pos, size );
        uint32_t maxlen = refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest2 >> LZAAHE_REFHASH_BITS;

        if (len2 < len1 && len1 >= maxlen) {
            refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest2 = pos | (len1 << LZAAHE_REFHASH_BITS);
        }

        if (len2 >= maxlen) {
            refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest1 = pos | (len2 << LZAAHE_REFHASH_BITS);
        }

        matchlength = len1 > len2 ? len1 : len2;

        return true;
    }
    else
    {
        // New sym
        if (i < LZAAHE_REFHASH_ENTITIES)
        {
            refhash[hash*LZAAHE_REFHASH_ENTITIES+i].sym = str4;
            refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest1 = pos;
            refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest2 = pos;
            refhashcount[hash]++;
        }

        return false;
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

    for (uint32_t i=0; i<256; i++)
        ctx->dict[i] = (1 << 8) | i;

    //lzaahe_bufferStats( ctx->dict, ctx->reverse_dictionnary, ctx->proba_tables, ctx->tmp_tables );

    for (uint32_t i=0; i<size; i++)
    {
        // lz string match search
        uint32_t str4 = *((uint32_t*) (ctx->inputBlock+i));
        uint32_t strhash = lzaahe_getHash(str4);

        // huff encode
        uint8_t sym = ctx->inputBlock[i];
        uint8_t current = ctx->reverse_dictionnary[sym];

        for (uint32_t j=0; j<8; j++)
        {
            arith_encodebit( ctx->arithEncoder, ctx->proba_tables[j][current&((1<<j)-1)], (current&(1<<j))>>j);
        }
    }

    arith_finalize( ctx->arithEncoder );

    ctx->outputSize += arith_getoutptr( ctx->arithEncoder );
}
