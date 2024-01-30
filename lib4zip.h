#pragma once


#include <cstdint>


struct BitIOCtx;


#define LZAAHE_BLOCK_SZ (1<<22)
#define LZAAHE_OUTPUT_SZ ((1<<22) + (1<<20))


enum LZAAHEDictEnum {
    LZAAHEDictOne = 0
};


enum LZAAHEHuff {
    LZAAHEDynamicHuff = 0
};


enum LZAAHEContextType {
    LZAAHECompress = 0,
    LZAAHEDecompress
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
    // Huff
    uint32_t *dict;
    uint8_t *reverse_dictionnary;
    uint32_t **proba_tables;
    uint32_t **tmp_tables;
    // I/O
    uint8_t *inputBlock;
    uint8_t *outputBlock;
    uint32_t outputSize;
    uint32_t inputSize;
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
    // I/O
    uint8_t *inputBlock;
    uint8_t *outputBlock;
    uint32_t outputSize;
    uint32_t inputSize;
    struct BitIOCtx *io;
};
#pragma pack()


#if defined (__cplusplus)
extern "C" {
#endif

    struct LZAAHECompressionContext* lzaaheAllocateCompression();
    void lzaaheDeallocateCompression(struct LZAAHECompressionContext* ctx);
    void lzaaheEncode( struct LZAAHECompressionContext* ctx );

    struct LZAAHEDecompressionContext* lzaaheAllocateDecompression();
    void lzaaheDeallocateDecompression(struct LZAAHEDecompressionContext* ctx);
    void lzaaheDecode( struct LZAAHEDecompressionContext* ctx );

#if defined (__cplusplus)
}
#endif

