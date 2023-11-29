#pragma once


#include <cstdint>


struct ArithCtx;
struct LZAAHEDict;


#define LZAAHE_BLOCK_SZ (1<<22)


enum LZAAHEDictEnum {
    LZAAHEDictNone = 0,
    LZAAHEDictOne,
    LZAAHEDictTwo,
    LZAAHELineOfSight
};


enum LZAAHEHuff {
    LZAAHEHuffNone = 0,
    LZAAHEHuffStd,
    LZAAHEHuffAdaptBlock,
    LZAAHEHuffAdaptBEnd,
    LZAAHEHuffFullAdaptative,
    LZAAHEProgressivePPM
};


struct LZAAHEOptions {
    uint8_t nPotRestartMarkers;
    uint8_t lzMethod;
    uint8_t huffMethod;
    bool useLzArith;
    bool useHArith;
};


struct LZAAHEDict;


struct LZAAHEContext {
    uint32_t *dict;
    uint8_t *reverse_dictionnary;
    uint32_t *stats;
    uint32_t **proba_tables;
    uint32_t **tmp_tables;
    uint8_t *inputBlock;
    uint8_t *outputBlock;
    uint32_t outputSize;
    uint32_t inputSize;
    struct ArithCtx *arithEncoder;
    struct LZAAHEDict *lzdict;
    struct LZAAHEOptions options;
};


extern "C" struct LZAAHEContext* allocateLZAAHEContext( uint32_t compressionLevel );
extern "C" void deallocateLZAAHEContext(struct LZAAHEContext* ctx);
extern "C" void lzaaheEncode( struct LZAAHEContext* ctx );
extern "C" void lzaaheDecode( struct LZAAHEContext* ctx );

