#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <unordered_map>



#include "lzaahe_encode.h"
#include "lzaahe_common.h"



static inline uint32_t matchlen( uint8_t *inbuff, uint32_t first, uint32_t second, uint32_t decoded_size, uint32_t size )
{
    uint32_t i = 4; // The first 4 bytes of both strings are already matching at this point

    while ((first+i) < decoded_size && (second+i) < size && i < 68 && inbuff[first+i] == inbuff[second+i]) i++;

    return i;
}


static inline void writebits( struct LZAAHEContext* ctx, uint32_t d, uint32_t bits )
{
    for (uint32_t i=0; i<bits; i++)
    {
        arith_encodebit( ctx->arithEncoder, (1<<(ARITH_PRECISION-1)), (d&(1<<i))>>i);
    }
}


static inline bool addHit(uint8_t *inbuff, uint32_t size, struct LZAAHEContext::SymRef *refhash, uint8_t *refhashcount, uint32_t hash, uint32_t str4, uint32_t pos, uint32_t &matchlength, uint32_t &hitpos, uint32_t &posid, uint32_t &currentposid)
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

        if (refhash[hash*LZAAHE_REFHASH_ENTITIES+i].hid == -1)
        {
            refhash[hash*LZAAHE_REFHASH_ENTITIES+i].hid = currentposid++;
        }

        posid = refhash[hash*LZAAHE_REFHASH_ENTITIES+i].hid;

        uint32_t len1 = matchlen( inbuff, refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest1 & (LZAAHE_BLOCK_SZ-1), pos, pos, size );
        uint32_t len2 = matchlen( inbuff, refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest2 & (LZAAHE_BLOCK_SZ-1), pos, pos, size );
        uint32_t maxlen = refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest2 >> LZAAHE_REFHASH_BITS;

        if (len2 < len1 && len1 >= maxlen) {
            refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest2 = pos | (len1 << LZAAHE_REFHASH_BITS);
        }

        if (len2 >= maxlen) {
            refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest1 = pos | (len2 << LZAAHE_REFHASH_BITS);
        }

        if (len1 > len2)
        {
            matchlength = len1;
            hitpos = refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest1 & (LZAAHE_BLOCK_SZ-1);
        }
        else
        {
            matchlength = len2;
            hitpos = refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest2 & (LZAAHE_BLOCK_SZ-1);
        }

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
            refhash[hash*LZAAHE_REFHASH_ENTITIES+i].hid = -1;
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


    uint32_t hitsave = 0;
    uint32_t n_hits = 0;
    uint32_t n_huffsyms = 0;
    bool hufftables = false;
    uint32_t i=0;
    uint32_t currentposid=0;
    uint32_t posid;


    uint32_t hitlenstat[256] = { 0 };


    while (i < size)
    {
        // lz string match search
        uint32_t str4 = *((uint32_t*) (ctx->inputBlock+i));
        uint32_t strhash = lzaahe_getHash(str4);
        uint32_t hitlen = 1;
        uint32_t hitpos = 0;

        if (addHit(ctx->inputBlock, size, ctx->refhash, ctx->refhashcount, strhash, str4, i, hitlen, hitpos, posid, currentposid))
        {
            bool islen4 = hitlen-4 == 0;

            arith_encodebit( ctx->arithEncoder, (1<<(ARITH_PRECISION-1)), 1);
            arith_encodebit( ctx->arithEncoder, (1<<(ARITH_PRECISION-1)), islen4);

            if (!islen4)
            {
                writebits( ctx, hitlen-4, 6 );
            }

            writebits( ctx, posid, lzaahe_potHigher(currentposid) );

            hitlenstat[hitlen-4]++;
            hitsave += hitlen;
            n_hits++;
        }
        else
        {
            uint8_t sym = ctx->inputBlock[i];

            arith_encodebit( ctx->arithEncoder, (1<<(ARITH_PRECISION-1)), 0);

            if ((n_huffsyms != 0) && (n_huffsyms & 0xFF) == 0)
            {
                lzaahe_bufferStats( ctx->dict, ctx->reverse_dictionnary, ctx->proba_tables, ctx->tmp_tables );
                hufftables = true;
            }

            if (hufftables)
            {
                uint8_t current = ctx->reverse_dictionnary[sym];

                for (uint32_t j=0; j<8; j++)
                {
                    arith_encodebit( ctx->arithEncoder, ctx->proba_tables[j][current&((1<<j)-1)], (current&(1<<j))>>j);
                }

                ctx->dict[current] += 256;
            }
            else
            {
                for (uint32_t j=0; j<8; j++)
                {
                    arith_encodebit( ctx->arithEncoder, (1<<(ARITH_PRECISION-1)), (sym&(1<<j))>>j);
                }

                ctx->dict[sym] += 256;
            }

            n_huffsyms++;
        }

        i += hitlen;
    }

    arith_finalize( ctx->arithEncoder );

    ctx->outputSize += arith_getoutptr( ctx->arithEncoder );
}
