#pragma once

/*
** Lib4zip general API include file.
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


#include <cstdint>


struct BitIOCtx;


#define LZAAHE_BLOCK_BITS (18)
#define LZAAHE_BLOCK_SZ (1<<LZAAHE_BLOCK_BITS)
#define LZAAHE_OUTPUT_SZ ((1<<LZAAHE_BLOCK_BITS) + (1<<(LZAAHE_BLOCK_BITS-2)))


enum LZAAHEDictEnum {
    LZAAHEDictOne = 0
};


enum LZAAHEHuff {
    LZAAHENoHuff = 0,
    LZAAHEStaticHuff
};


#pragma pack(1)
struct LZAAHEOptions {
    uint8_t nPotRestartMarkers;
    uint8_t lzMethod;
    uint8_t huffMethod;
};


struct LZAAHECompressionContext {
    // LZ string matching
    struct SymRef {
        uint32_t sym;
        uint32_t pos;
        uint32_t hit_id;
        uint16_t matchlen;
        uint16_t n_occurences;
    };
    struct SymRef *refhash;
    uint8_t *refhashcount;
    // RLE uncompressed/symbols
    uint64_t *hits;
    // non-matching symbols
    uint8_t *uncompressed_syms;
    // LZ matches
    uint32_t *lz_matches;
    uint32_t id;
    uint32_t id_mask;
    uint32_t id_bits;
    struct LZAAHEOptions options;
};


struct LZAAHEDecompressionContext {
    struct Symbol {
        uint32_t pos;
        uint32_t len;
    };
    struct Symbol* symlist;
    struct BitIOCtx *io;
};
#pragma pack()


#if defined (__cplusplus)
extern "C" {
#endif

    struct LZAAHECompressionContext* lzaaheAllocateCompression();
    void lzaaheDeallocateCompression(struct LZAAHECompressionContext* ctx);
    void lzaaheEncode( struct LZAAHECompressionContext* ctx, uint8_t *inputBlock, uint8_t *outputBlock, uint32_t *outputSize, uint32_t inputSize );

    struct LZAAHEDecompressionContext* lzaaheAllocateDecompression();
    void lzaaheDeallocateDecompression(struct LZAAHEDecompressionContext* ctx);
    void lzaaheDecode( struct LZAAHEDecompressionContext* ctx, uint8_t *inputBlock, uint8_t *outputBlock, uint32_t *outputSize, uint32_t inputSize );

#if defined (__cplusplus)
}
#endif

