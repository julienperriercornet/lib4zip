#pragma once


#include <cstdint>


struct BitIOCtx;


#define LZAAHE_BLOCK_BITS (22)
#define LZAAHE_BLOCK_SZ (1<<LZAAHE_BLOCK_BITS)
#define LZAAHE_OUTPUT_SZ ((1<<LZAAHE_BLOCK_BITS) + (1<<LZAAHE_BLOCK_BITS-2))


enum LZAAHEDictEnum {
    LZAAHEDictOne = 0
};


enum LZAAHEHuff {
    LZAAHENoHuff = 0,
    LZAAHEDynamicHuff
};


#pragma pack(1)
struct LZAAHEOptions {
    uint8_t nPotRestartMarkers;
    uint8_t lzMethod;
    uint8_t huffMethod;
};


struct LZAAHECompressionContext {
    // LZ
    struct SymRef {
        uint32_t sym;
        uint32_t longest;
        uint32_t hit_id;
        uint16_t matchlen;
        uint16_t n_occurences;
    };
    struct SymRef *refhash;
    uint8_t *refhashcount;
    struct BitIOCtx *io;
    struct RefCnt {
        uint32_t id_cnt;
        uint32_t id_bits;
        uint32_t id_mask;
    } refcount;
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

