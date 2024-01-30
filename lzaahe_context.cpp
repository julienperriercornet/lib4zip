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
        context->io = nullptr;

        context->options = getLZAAHEOptions( 10 );

        context->refhash = (struct LZAAHECompressionContext::SymRef*) align_alloc( MAX_CACHE_LINE_SIZE, LZAAHE_REFHASH_SZ*LZAAHE_REFHASH_ENTITIES*sizeof(struct LZAAHECompressionContext::SymRef) );
        context->refhashcount = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, LZAAHE_REFHASH_SZ*sizeof(uint8_t) );

        context->io = (struct BitIOCtx*) align_alloc( MAX_CACHE_LINE_SIZE, sizeof(struct BitIOCtx) );

        if (context->refhash == nullptr || context->refhashcount == nullptr ||context->io == nullptr)
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
    if (ctx->io != nullptr) align_free(ctx->io);
    align_free( ctx );
}


extern "C" struct LZAAHEDecompressionContext* lzaaheAllocateDecompression()
{
    struct LZAAHEDecompressionContext* context = (struct LZAAHEDecompressionContext*) align_alloc( MAX_CACHE_LINE_SIZE, sizeof(struct LZAAHEDecompressionContext) );

    if (context)
    {
        context->symlist = (struct LZAAHEDecompressionContext::Symbol*) align_alloc( MAX_CACHE_LINE_SIZE, LZAAHE_MAX_SYMBOLS*sizeof(struct LZAAHEDecompressionContext::Symbol) );

        context->io = (struct BitIOCtx*) align_alloc( MAX_CACHE_LINE_SIZE, sizeof(struct BitIOCtx) );

        if (context->symlist == nullptr || context->io == nullptr)
        {
            lzaaheDeallocateDecompression( context );
            context = nullptr;
        }
    }

    return context;
}

