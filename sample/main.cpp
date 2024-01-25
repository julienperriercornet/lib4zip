#include <cstdio>
#include <cstdlib>
#include <time.h>


#include "../lib4zip.h"


void compress( FILE* in, FILE* out )
{
    struct LZAAHECompressionContext* ctx = lzaaheAllocateCompression();

    if (ctx)
    {

        fseek( in, 0, SEEK_END );
        size_t remainsz = ftell( in );
        fseek( in, 0, SEEK_SET );

        size_t to_read = remainsz > LZAAHE_BLOCK_SZ ? LZAAHE_BLOCK_SZ : remainsz;

        while ( to_read > 0 && to_read == fread( ctx->inputBlock, 1, to_read, in ) )
        {
            ctx->inputSize = to_read;
            lzaaheEncode( ctx );
            fputc(ctx->outputSize & 0xFF, out);
            fputc(((ctx->outputSize >> 8) & 0xFF), out);
            fputc(((ctx->outputSize >> 16) & 0xFF), out);
            fwrite( ctx->outputBlock, 1, ctx->outputSize, out );
            remainsz -= to_read;
            to_read = remainsz > LZAAHE_BLOCK_SZ ? LZAAHE_BLOCK_SZ : remainsz;
        }

        lzaaheDeallocateCompression(ctx);
    }
}


void decompress( FILE* in, FILE* out )
{
    struct LZAAHEDecompressionContext* ctx = lzaaheAllocateDecompression();

    if (ctx)
    {

        uint32_t to_read = fgetc(in);
        to_read += fgetc(in) << 8;
        to_read += fgetc(in) << 16;

        while ( !feof(in) && to_read > 0 &&
            to_read < LZAAHE_OUTPUT_SZ &&
            to_read == fread( ctx->inputBlock, 1, to_read, in ) )
        {
            ctx->inputSize = to_read;
            lzaaheDecode( ctx );
            fwrite( ctx->outputBlock, 1, ctx->outputSize, out );

            to_read = fgetc(in);
            to_read += fgetc(in) << 8;
            to_read += fgetc(in) << 16;
        }

        lzaaheDeallocateDecompression(ctx);
    }
}


int main( int argc, const char** argv )
{
    if (argc != 4)
    {
        printf("lib4zip v0.1 alpha\n"
        "(C) 2022-2023, Julien Perrier-cornet. Free software under GPLv3 Licence.\n"
        "\n"
        "To compress/decompress: samplelib4zip c/d input output\n");
        return 1;
    }

    FILE *in = fopen(argv[2], "rb");
    if (!in) return 1;

    FILE *out = fopen(argv[3], "wb");
    if (!out) return 1;

    clock_t start = clock();

    if (argv[1][0] == 'c')
        compress(in, out);
    else if (argv[1][0] == 'd')
        decompress(in, out);

    printf("%s (%ld) -> %s (%ld) in %.3fs\n",
        argv[2], ftell(in), argv[3], ftell(out),
        double(clock()-start)/CLOCKS_PER_SEC);

    return 0;
}

