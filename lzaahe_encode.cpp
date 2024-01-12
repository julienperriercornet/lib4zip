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

    while ((first+i) < decoded_size && (second+i) < size && i < 72 && inbuff[first+i] == inbuff[second+i]) i++;

    return i;
}


static inline void writebits( struct LZAAHEContext* ctx, uint32_t d, uint32_t bits )
{
    bitio_write( ctx->io, bits, d);
}


static inline bool addHit(uint8_t *inbuff, uint32_t size, struct LZAAHEContext::SymRef *refhash, uint8_t *refhashcount, uint32_t hash, uint32_t str4, uint32_t pos, uint32_t &matchlength, uint32_t &hitidx, uint32_t &hitpos, bool use2str)
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

        hitidx = hash*LZAAHE_REFHASH_ENTITIES+i;
        hitpos = refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest1 & (LZAAHE_BLOCK_SZ-1);

        uint32_t len1 = matchlen( inbuff, refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest1 & (LZAAHE_BLOCK_SZ-1), pos, pos, size );
        uint32_t len2 = 0;
        if (use2str)
            len2 = matchlen( inbuff, refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest2 & (LZAAHE_BLOCK_SZ-1), pos, pos, size );
        uint32_t maxlen = refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest2 >> LZAAHE_REFHASH_BITS;

        if (len2 < len1 && len1 >= maxlen) {
            refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest2 = pos | (len1 << LZAAHE_REFHASH_BITS);
        }

        if (len2 >= maxlen) {
            refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest1 = pos | (len2 << LZAAHE_REFHASH_BITS);
            refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest2 = (refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest2 & (LZAAHE_BLOCK_SZ-1)) | (len2 << LZAAHE_REFHASH_BITS);
        }

        if (len1 > len2)
        {
            matchlength = len1;
        }
        else
        {
            matchlength = len2;
        }

        return true;
    }
    else if (i >= refhashcount[hash] && i < LZAAHE_REFHASH_ENTITIES)
    {
        refhash[hash*LZAAHE_REFHASH_ENTITIES+i].sym = str4;
        refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest1 = pos;
        refhash[hash*LZAAHE_REFHASH_ENTITIES+i].longest2 = pos;
        refhash[hash*LZAAHE_REFHASH_ENTITIES+i].hit_id_4 = -1;
        refhash[hash*LZAAHE_REFHASH_ENTITIES+i].hit_id_4_c1 = 0xFFFF;
        refhash[hash*LZAAHE_REFHASH_ENTITIES+i].hit_id_4_c2 = 0xFFFF;
        refhash[hash*LZAAHE_REFHASH_ENTITIES+i].hit_id = -1;
        refhash[hash*LZAAHE_REFHASH_ENTITIES+i].hit_id_c1 = 0xFFFF;
        refhash[hash*LZAAHE_REFHASH_ENTITIES+i].hit_id_c2 = 0xFFFF;

        refhashcount[hash]++;
    }

    return false;
}


static void makeL1L2cache( struct LZAAHEContext* ctx )
{
    std::vector<uint32_t> syms4;
    std::vector<uint32_t> symsany;

    for (uint32_t i=0; i<LZAAHE_REFHASH_SZ; i++)
    {
        for (uint32_t j=0; j<ctx->refhashcount[i]; j++)
        {
            if (ctx->refhash[i*LZAAHE_REFHASH_ENTITIES+j].longest1 >= LZAAHE_REFHASH_SZ)
            {
                uint32_t n_hits = ctx->refhash[i*LZAAHE_REFHASH_ENTITIES+j].longest1 / LZAAHE_REFHASH_SZ;

                if (ctx->refhash[i*LZAAHE_REFHASH_ENTITIES+j].hit_id_4 != -1)
                {
                    syms4.push_back( (i*LZAAHE_REFHASH_ENTITIES+j) | (n_hits*LZAAHE_REFHASH_SZ*LZAAHE_REFHASH_ENTITIES) );
                    ctx->refhash[i*LZAAHE_REFHASH_ENTITIES+j].hit_id_4_c1 = 0xFFFF;
                    ctx->refhash[i*LZAAHE_REFHASH_ENTITIES+j].hit_id_4_c2 = 0xFFFF;
                }

                if (ctx->refhash[i*LZAAHE_REFHASH_ENTITIES+j].hit_id != -1)
                {
                    symsany.push_back( (i*LZAAHE_REFHASH_ENTITIES+j) | (n_hits*LZAAHE_REFHASH_SZ*LZAAHE_REFHASH_ENTITIES) );
                    ctx->refhash[i*LZAAHE_REFHASH_ENTITIES+j].hit_id_c1 = 0xFFFF;
                    ctx->refhash[i*LZAAHE_REFHASH_ENTITIES+j].hit_id_c2 = 0xFFFF;
                }
            }
        }
    }

    std::sort(syms4.begin(), syms4.end());
    std::sort(symsany.begin(), symsany.end());

    uint32_t syms4sz = syms4.size();
    uint32_t nsyms4 = syms4.size() < (256 + 4096) ? syms4.size() : (256 + 4096);
    uint32_t syms4start = syms4.size() - nsyms4;

    if (nsyms4 == syms4.size())
    {
        if (nsyms4 < 256)
            ctx->refcount.len4_c1_bits = lzaahe_potHigher(nsyms4);
        if ((nsyms4 - 256) < 4096)
            ctx->refcount.len4_c2_bits = lzaahe_potHigher(nsyms4 - 256);
    }
    else
    {
        ctx->refcount.len4_c1_bits = 8;
        ctx->refcount.len4_c2_bits = 12;
    }

    for (uint32_t i=syms4start; i<syms4sz; i++)
    {
        if (i >= (syms4.size() - 256))
            ctx->refhash[syms4[i] & ((1<<(LZAAHE_REFHASH_BITS+1))-1)].hit_id_c1 = syms4.size() - i + 1;
        else
            ctx->refhash[syms4[i] & ((1<<(LZAAHE_REFHASH_BITS+1))-1)].hit_id_c2 = syms4.size() - 256 - i + 1;
    }

    uint32_t symssz = symsany.size();
    uint32_t nsyms = symsany.size() < (256 + 4096) ? symsany.size() : (256 + 4096);
    uint32_t symsstart = symsany.size() - nsyms;

    if (nsyms == symsany.size())
    {
        if (nsyms < 256)
            ctx->refcount.any_c1_bits = lzaahe_potHigher(nsyms);
        if ((nsyms - 256) < 4096)
            ctx->refcount.any_c2_bits = lzaahe_potHigher(nsyms - 256);
    }
    else
    {
        ctx->refcount.any_c1_bits = 8;
        ctx->refcount.any_c2_bits = 12;
    }

    for (uint32_t i=symsstart; i<symssz; i++)
    {
        if (i >= (symsany.size() - 256))
            ctx->refhash[symsany[i] & ((1<<(LZAAHE_REFHASH_BITS+1))-1)].hit_id_c1 = symsany.size() - i + 1;
        else
            ctx->refhash[symsany[i] & ((1<<(LZAAHE_REFHASH_BITS+1))-1)].hit_id_c2 = symsany.size() - 256 - i + 1;
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

    bitio_init( ctx->io, ctx->outputBlock+3, LZAAHE_OUTPUT_SZ-3 );

    for (uint32_t i=0; i<256; i++)
        ctx->dict[i] = (1 << 8) | i;

    uint32_t hitsave = 0;
    uint32_t n_hitany = 0;
    uint32_t n_hitlen4 = 0;
    uint32_t n_huffsyms = 0;
    bool hufftables = false;
    uint32_t i = 0;
    uint32_t n_updates = 0;

    uint32_t hitlenstat[256] = { 0 };

    while (i < size)
    {
        // lz string match search
        uint32_t str4 = 0;
        uint32_t strhash = 0;
        uint32_t hitlen = 1;
        uint32_t hitidx = -1;
        uint32_t hitpos = -1;

        if ((ctx->options.lzMethod == LZAAHEDictTwoL1L2) && ((i+16384) >> 19) > n_updates )
        {
            makeL1L2cache( ctx );
            n_updates++;
        }

        if (i < size-3)
        {
            str4 = *((uint32_t*) (ctx->inputBlock+i));
            strhash = lzaahe_getHash(str4);
        }

        if ((i < size-3) && addHit(ctx->inputBlock, size, ctx->refhash, ctx->refhashcount,
            strhash, str4, i, hitlen, hitidx, hitpos, ctx->options.lzMethod != LZAAHEDictOne))
        {
            bool islen4 = hitlen == 4;

            bitio_write( ctx->io, 1);
            bitio_write( ctx->io, islen4);

            if (!islen4)
            {
                bitio_write( ctx->io, hitlen>=9 );

                if (hitlen<9) writebits( ctx, hitlen-5, 2 );
                else writebits( ctx, hitlen-9, 6 );
            }

            if (islen4)
            {
                if (ctx->refhash[hitidx].hit_id_4_c1 != 0xFFFF) {
                    writebits( ctx, 0, 2 );
                    writebits( ctx, ctx->refhash[hitidx].hit_id_4_c1, ctx->refcount.len4_c1_bits );
                }
                else if (ctx->refhash[hitidx].hit_id_4_c2 != 0xFFFF) {
                    writebits( ctx, 1, 2 );
                    writebits( ctx, ctx->refhash[hitidx].hit_id_4_c2, ctx->refcount.len4_c2_bits );
                }
                else
                {
                    if (ctx->refhash[hitidx].hit_id_4 == -1) {
                        ctx->refhash[hitidx].hit_id_4 = ctx->refcount.len4_id_cnt++;
                        writebits( ctx, 3, 2 );
                        writebits( ctx, hitpos, lzaahe_potHigher(i) );
                    }
                    else {
                        writebits( ctx, 2, 2 );
                        writebits( ctx, ctx->refhash[hitidx].hit_id_4, ctx->refcount.len4_id_bits );
                    }

                    if (ctx->refcount.len4_id_cnt != 0 && (ctx->refcount.len4_id_cnt & ctx->refcount.len4_id_mask) == 0) {
                        ctx->refcount.len4_id_bits++;
                        ctx->refcount.len4_id_mask = (1 << ctx->refcount.len4_id_bits) - 1;
                    }
                }

                n_hitlen4++;
            }
            else
            {
                if (ctx->refhash[hitidx].hit_id_c1 != 0xFFFF) {
                    writebits( ctx, 0, 2 );
                    writebits( ctx, ctx->refhash[hitidx].hit_id_c1, ctx->refcount.any_c1_bits );
                }
                else if (ctx->refhash[hitidx].hit_id_c2 != 0xFFFF) {
                    writebits( ctx, 1, 2 );
                    writebits( ctx, ctx->refhash[hitidx].hit_id_c2, ctx->refcount.any_c2_bits );
                }
                else
                {
                    if (ctx->refhash[hitidx].hit_id == -1) {
                        ctx->refhash[hitidx].hit_id = ctx->refcount.any_id_cnt++;
                        writebits( ctx, 3, 2 );
                        writebits( ctx, hitpos, lzaahe_potHigher(i) );
                    }
                    else {
                        writebits( ctx, 2, 2 );
                        writebits( ctx, ctx->refhash[hitidx].hit_id, ctx->refcount.any_id_bits );
                    }

                    if (ctx->refcount.any_id_cnt != 0 && (ctx->refcount.any_id_cnt & ctx->refcount.any_id_mask) == 0) {
                        ctx->refcount.any_id_bits++;
                        ctx->refcount.any_id_mask = (1 << ctx->refcount.any_id_bits) - 1;
                    }
                }

                n_hitany++;
            }

            hitlenstat[hitlen-4]++;
            hitsave += hitlen;
        }
        else
        {
            uint8_t sym = ctx->inputBlock[i];

            bitio_write( ctx->io, 0);
            writebits( ctx, sym, 8 );

            n_huffsyms++;
        }

        i += hitlen;
    }

    bitio_finalize( ctx->io );

    ctx->outputSize += bitio_getoutptr( ctx->io );
}

