#include <cstdio>
#include <cstdlib>


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
            options.lzMethod = 1;
            options.huffMethod = 1;
            options.useLzArith = true;
            options.useHArith = true;
            break;
    }

    return options;
}


extern "C" void deallocateLZAAHEContext( LZAAHEContext* ctx )
{
    if (ctx->dict != nullptr) free(ctx->dict);
    if (ctx->reverse_dictionnary != nullptr) free(ctx->reverse_dictionnary);
    if (ctx->stats != nullptr) free(ctx->stats);
    if (ctx->proba_tables != nullptr && ctx->proba_tables[0] != nullptr) free(ctx->proba_tables[0]);
    if (ctx->proba_tables != nullptr) free(ctx->proba_tables);
    if (ctx->inputBlock != nullptr) free(ctx->inputBlock);
    if (ctx->outputBlock != nullptr) free(ctx->outputBlock);
    if (ctx->arithEncoder != nullptr) free(ctx->arithEncoder);
    free( ctx );
}


extern "C" LZAAHEContext* allocateLZAAHEContext( uint32_t compressionLevel )
{
    struct LZAAHEContext* context = (struct LZAAHEContext*) aligned_alloc( 256, sizeof(struct LZAAHEContext) );

    if (context)
    {
        context->dict = nullptr;
        context->reverse_dictionnary = nullptr;
        context->stats = nullptr;
        context->proba_tables = nullptr;
        context->inputBlock = nullptr;
        context->outputBlock = nullptr;
        context->arithEncoder = nullptr;

        context->options = getLZAAHEOptions( compressionLevel );

        context->dict = (uint32_t*) aligned_alloc( 256, 256*sizeof(uint32_t) );
        context->reverse_dictionnary = (uint8_t*) aligned_alloc( 256, 256*sizeof(uint8_t) );
        context->stats = (uint32_t*) aligned_alloc( 256, 256*sizeof(uint32_t) );
        context->proba_tables = (uint32_t**) aligned_alloc( 256, 8*sizeof(uint32_t*) );
        if (context->proba_tables)
        {
            context->proba_tables[0] = (uint32_t*) aligned_alloc( 256, 256*sizeof(uint32_t) );
            context->proba_tables[1] = context->proba_tables[0] + 1;
            context->proba_tables[2] = context->proba_tables[0] + 3;
            context->proba_tables[3] = context->proba_tables[0] + 7;
            context->proba_tables[4] = context->proba_tables[0] + 15;
            context->proba_tables[5] = context->proba_tables[0] + 31;
            context->proba_tables[6] = context->proba_tables[0] + 63;
            context->proba_tables[7] = context->proba_tables[0] + 127;
        }
        context->inputBlock = (uint8_t*) aligned_alloc( 256, LZAAHE_OUTPUT_SZ*sizeof(uint8_t) );
        context->outputBlock = (uint8_t*) aligned_alloc( 256, LZAAHE_OUTPUT_SZ*sizeof(uint8_t) );
        context->arithEncoder = (struct ArithCtx*) aligned_alloc( 256, sizeof(struct ArithCtx) );

        if (context->dict == nullptr || context->reverse_dictionnary == nullptr || context->stats == nullptr ||
            context->proba_tables == nullptr || context->proba_tables[0] == nullptr || context->inputBlock == nullptr ||
            context->outputBlock == nullptr || context->arithEncoder == nullptr)
        {
            deallocateLZAAHEContext( context );
            context = nullptr;
        }
    }

    return context;
}
