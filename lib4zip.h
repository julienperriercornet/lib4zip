#pragma once


#include <cstdint>


struct BitIOCtx;


#define LZAAHE_BLOCK_SZ (1<<19)
#define LZAAHE_OUTPUT_SZ ((1<<19) + (1<<17))


enum LZAAHEDictEnum {
    LZAAHEDictOne = 0,
    LZAAHEDictTwo
};


enum LZAAHEHuff {
    LZAAHEDynamicHuff = 0
};


#pragma pack(1)
struct LZAAHEOptions {
    uint8_t nPotRestartMarkers;
    uint8_t lzMethod;
    uint8_t huffMethod;
};


struct LZAAHEContext {
    // LZ
    struct SymRef {
        uint32_t sym;
        uint32_t longest1;
        uint32_t longest2;
        uint32_t hid;
    };
    uint32_t *bytehashcount;
    uint32_t *bytehash;
    uint32_t *ringbuffer;
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
    struct LZAAHEOptions options;
};
#pragma pack()


extern "C" struct LZAAHEContext* lzaaheAllocate( uint32_t compressionLevel );
extern "C" void lzaaheDeallocate(struct LZAAHEContext* ctx);
extern "C" void lzaaheInit(struct LZAAHEContext* ctx);
extern "C" void lzaaheEncode( struct LZAAHEContext* ctx );
extern "C" void lzaaheDecode( struct LZAAHEContext* ctx );

