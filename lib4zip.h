#pragma once


#include <cstdint>


struct BitIOCtx;


#define LZAAHE_BLOCK_SZ (1<<22)
#define LZAAHE_OUTPUT_SZ ((1<<22) + (1<<20))


enum LZAAHEDictEnum {
    LZAAHEDictOne = 0,
    LZAAHEDictTwo,
    LZAAHEDictTwoL1L2
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
        uint32_t hit_id_4;
        uint16_t hit_id_4_c1, hit_id_4_c2;
        uint32_t hit_id;
        uint16_t hit_id_c1, hit_id_c2;
    };
    struct RefCnt {
        uint32_t len4_id_cnt;
        uint32_t len4_c1_cnt;
        uint32_t len4_c2_cnt;
        uint32_t any_id_cnt;
        uint32_t any_c1_cnt;
        uint32_t any_c2_cnt;
        uint32_t len4_id_bits;
        uint32_t len4_id_mask;
        uint32_t len4_c1_bits;
        uint32_t len4_c1_mask;
        uint32_t len4_c2_bits;
        uint32_t len4_c2_mask;
        uint32_t any_id_bits;
        uint32_t any_id_mask;
        uint32_t any_c1_bits;
        uint32_t any_c1_mask;
        uint32_t any_c2_bits;
        uint32_t any_c2_mask;
    } refcount;
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

