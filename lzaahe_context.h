#pragma once


#include "arith.h"
#include "platform.h"


#define LZAAHE_BLOCK_SZ (1<<22)
#define LZAAHE_OUTPUT_SZ ((1<<22) + (1<<19))


struct LZAAHEOptions {
    uint8_t nPotRestartMarkers;
    uint8_t lzMethod;
    uint8_t huffMethod;
    bool useLzArith;
    bool useHArith;
};


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
    struct LZAAHEOptions options;
};


extern "C" LZAAHEContext* allocateLZAAHEContext( uint32_t compressionLevel );
extern "C" void deallocateLZAAHEContext(LZAAHEContext* ctx);

