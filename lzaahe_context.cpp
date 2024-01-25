#include <cstdio>
#include <cstdlib>
#include <cstring>


#include "lzaahe_context.h"


static struct LZAAHEOptions getLZAAHEOptions( uint32_t compressionLevel )
{
    struct LZAAHEOptions options;

    // We don't use restart markers/multithreading in this early version
    options.nPotRestartMarkers = 0;

    switch (compressionLevel)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        default:
            options.lzMethod = LZAAHEDictOne;
            options.huffMethod = LZAAHEDynamicHuff;
            break;
    }

    return options;
}


extern "C" void lzaaheDeallocateCompression( struct LZAAHECompressionContext* ctx )
{
    if (ctx->refhash != nullptr) align_free(ctx->refhash);
    if (ctx->refhashcount != nullptr) align_free(ctx->refhashcount);
    if (ctx->dict != nullptr) align_free(ctx->dict);
    if (ctx->reverse_dictionnary != nullptr) align_free(ctx->reverse_dictionnary);
    if (ctx->proba_tables != nullptr && ctx->proba_tables[0] != nullptr) align_free(ctx->proba_tables[0]);
    if (ctx->proba_tables != nullptr) align_free(ctx->proba_tables);
    if (ctx->tmp_tables != nullptr && ctx->tmp_tables[0] != nullptr) align_free(ctx->tmp_tables[0]);
    if (ctx->tmp_tables != nullptr) align_free(ctx->tmp_tables);
    if (ctx->inputBlock != nullptr) align_free(ctx->inputBlock);
    if (ctx->outputBlock != nullptr) align_free(ctx->outputBlock);
    if (ctx->io != nullptr) align_free(ctx->io);
    align_free( ctx );
}


extern "C" struct LZAAHECompressionContext* lzaaheAllocateCompression()
{
    struct LZAAHECompressionContext* context = (struct LZAAHECompressionContext*) align_alloc( MAX_CACHE_LINE_SIZE, sizeof(struct LZAAHECompressionContext) );

    if (context)
    {
        context->refhash = nullptr;
        context->refhashcount = nullptr;
        context->dict = nullptr;
        context->reverse_dictionnary = nullptr;
        context->proba_tables = nullptr;
        context->tmp_tables = nullptr;
        context->inputBlock = nullptr;
        context->outputBlock = nullptr;
        context->io = nullptr;

        context->options = getLZAAHEOptions( 10 );

        context->refhash = (struct LZAAHECompressionContext::SymRef*) align_alloc( MAX_CACHE_LINE_SIZE, LZAAHE_REFHASH_SZ*LZAAHE_REFHASH_ENTITIES*sizeof(struct LZAAHECompressionContext::SymRef) );
        context->refhashcount = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, LZAAHE_REFHASH_SZ*sizeof(uint8_t) );
        context->dict = (uint32_t*) align_alloc( MAX_CACHE_LINE_SIZE, 256*sizeof(uint32_t) );
        context->reverse_dictionnary = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, 256*sizeof(uint8_t) );
        context->proba_tables = (uint32_t**) align_alloc( MAX_CACHE_LINE_SIZE, 8*sizeof(uint32_t*) );
        if (context->proba_tables)
        {
            context->proba_tables[0] = (uint32_t*) align_alloc( MAX_CACHE_LINE_SIZE, 256*sizeof(uint32_t) );
            context->proba_tables[1] = context->proba_tables[0] + 1;
            context->proba_tables[2] = context->proba_tables[0] + 3;
            context->proba_tables[3] = context->proba_tables[0] + 7;
            context->proba_tables[4] = context->proba_tables[0] + 15;
            context->proba_tables[5] = context->proba_tables[0] + 31;
            context->proba_tables[6] = context->proba_tables[0] + 63;
            context->proba_tables[7] = context->proba_tables[0] + 127;
        }
        context->tmp_tables = (uint32_t**) align_alloc( MAX_CACHE_LINE_SIZE, 8*sizeof(uint32_t*) );
        if (context->tmp_tables)
        {
            context->tmp_tables[0] = (uint32_t*) align_alloc( MAX_CACHE_LINE_SIZE, 256*sizeof(uint32_t) );
            context->tmp_tables[1] = context->tmp_tables[0] + 1;
            context->tmp_tables[2] = context->tmp_tables[0] + 3;
            context->tmp_tables[3] = context->tmp_tables[0] + 7;
            context->tmp_tables[4] = context->tmp_tables[0] + 15;
            context->tmp_tables[5] = context->tmp_tables[0] + 31;
            context->tmp_tables[6] = context->tmp_tables[0] + 63;
            context->tmp_tables[7] = context->tmp_tables[0] + 127;
        }

        context->inputBlock = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, LZAAHE_OUTPUT_SZ*sizeof(uint8_t) );
        context->outputBlock = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, LZAAHE_OUTPUT_SZ*sizeof(uint8_t) );

        context->io = (struct BitIOCtx*) align_alloc( MAX_CACHE_LINE_SIZE, sizeof(struct BitIOCtx) );

        if (context->refhash == nullptr || context->refhashcount == nullptr ||
            context->dict == nullptr || context->reverse_dictionnary == nullptr ||
            context->tmp_tables == nullptr || context->tmp_tables[0] == nullptr ||
            context->proba_tables == nullptr || context->proba_tables[0] == nullptr || context->inputBlock == nullptr ||
            context->outputBlock == nullptr || context->io == nullptr)
        {
            lzaaheDeallocateCompression( context );
            context = nullptr;
        }
    }

    return context;
}


extern "C" void lzaaheDeallocateDecompression( struct LZAAHEDecompressionContext* ctx )
{
    if (ctx->symlist != nullptr) align_free(ctx->symlist);
    if (ctx->inputBlock != nullptr) align_free(ctx->inputBlock);
    if (ctx->outputBlock != nullptr) align_free(ctx->outputBlock);
    if (ctx->io != nullptr) align_free(ctx->io);
    align_free( ctx );
}


extern "C" struct LZAAHEDecompressionContext* lzaaheAllocateDecompression()
{
    struct LZAAHEDecompressionContext* context = (struct LZAAHEDecompressionContext*) align_alloc( MAX_CACHE_LINE_SIZE, sizeof(struct LZAAHEDecompressionContext) );

    if (context)
    {
        context->symlist = (struct LZAAHEDecompressionContext::Symbol*) align_alloc( MAX_CACHE_LINE_SIZE, LZAAHE_MAX_SYMBOLS*sizeof(struct LZAAHEDecompressionContext::Symbol) );

        context->inputBlock = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, LZAAHE_OUTPUT_SZ*sizeof(uint8_t) );
        context->outputBlock = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, LZAAHE_OUTPUT_SZ*sizeof(uint8_t) );

        context->io = (struct BitIOCtx*) align_alloc( MAX_CACHE_LINE_SIZE, sizeof(struct BitIOCtx) );

        if (context->symlist == nullptr || context->io == nullptr || context->inputBlock == nullptr ||  context->outputBlock == nullptr)
        {
            lzaaheDeallocateDecompression( context );
            context = nullptr;
        }
    }

    return context;
}

