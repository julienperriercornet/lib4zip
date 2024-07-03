/*
** Lib4zip sample.
** Copyright (C) 2022-2024 Julien Perrier-cornet
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstdio>
#include <cstdlib>
#include <time.h>


#include "../lib4zip.h"
#include "../platform.h"


void compress( FILE* in, FILE* out )
{
    uint8_t* inbuff = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, LZAAHE_BLOCK_SZ*sizeof(uint8_t) );
    uint8_t* outbuff = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, LZAAHE_OUTPUT_SZ*sizeof(uint8_t) );

    struct LZAAHECompressionContext* ctx = lzaaheAllocateCompression();

    if (ctx && inbuff && outbuff)
    {
        fseek( in, 0, SEEK_END );
        size_t remainsz = ftell( in );
        fseek( in, 0, SEEK_SET );

        size_t to_read = remainsz > LZAAHE_BLOCK_SZ ? LZAAHE_BLOCK_SZ : remainsz;

        while ( to_read > 0 && to_read == fread( inbuff, 1, to_read, in ) )
        {
            uint32_t outputSize;

            lzaaheEncode( ctx, inbuff, outbuff, &outputSize, to_read );

            fputc(outputSize & 0xFF, out);
            fputc(((outputSize >> 8) & 0xFF), out);
            fputc(((outputSize >> 16) & 0xFF), out);
            fwrite( outbuff, 1, outputSize, out );
            remainsz -= to_read;
            to_read = remainsz > LZAAHE_BLOCK_SZ ? LZAAHE_BLOCK_SZ : remainsz;
        }

    }

    if (ctx) lzaaheDeallocateCompression(ctx);

    if (outbuff != nullptr) align_free(outbuff);
    if (inbuff != nullptr) align_free(inbuff);
}


void compressLos( FILE* in, FILE* out )
{
    struct LOSCompressionContext* ctx = losAllocateContext();

    if (ctx)
    {
        fseek( in, 0, SEEK_END );
        size_t filesize = ftell( in );
        fseek( in, 0, SEEK_SET );

        uint8_t* inbuff = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, filesize*sizeof(uint8_t) );
        uint8_t* outbuff = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, (filesize+filesize/8)*sizeof(uint8_t) );

        if ( inbuff && outbuff && filesize > 0 && filesize == fread( inbuff, 1, filesize, in ) )
        {
            uint32_t outputSize;

            losEncode( ctx, inbuff, outbuff, &outputSize, filesize );

            fputc(filesize & 0xFF, out);
            fputc(((filesize >> 8) & 0xFF), out);
            fputc(((filesize >> 16) & 0xFF), out);
            fputc(((filesize >> 24) & 0xFF), out);

            fwrite( outbuff, 1, outputSize, out );
        }

        if (outbuff != nullptr) align_free(outbuff);
        if (inbuff != nullptr) align_free(inbuff);
    }

    if (ctx) losDeallocateContext(ctx);
}


void decompress( FILE* in, FILE* out )
{
    uint8_t* inbuff = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, LZAAHE_OUTPUT_SZ*sizeof(uint8_t) );
    uint8_t* outbuff = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, LZAAHE_BLOCK_SZ*sizeof(uint8_t) );

    struct LZAAHEDecompressionContext* ctx = lzaaheAllocateDecompression();

    if (ctx)
    {
        do
        {
            uint32_t to_read = fgetc(in);
            to_read += fgetc(in) << 8;
            to_read += fgetc(in) << 16;

            if (to_read > 0 && to_read < LZAAHE_OUTPUT_SZ && to_read == fread( inbuff, 1, to_read, in ))
            {
                uint32_t outputSize;

                lzaaheDecode( ctx, inbuff, outbuff, &outputSize, to_read );

                fwrite( outbuff, 1, outputSize, out );
            }
        }
        while ( !feof(in) ) ;
    }

    if (ctx) lzaaheDeallocateDecompression(ctx);

    if (outbuff != nullptr) align_free(outbuff);
    if (inbuff != nullptr) align_free(inbuff);
}


void decompressLos( FILE* in, FILE* out )
{
    struct LOSCompressionContext* ctx = losAllocateContext();

    if (ctx)
    {
        fseek( in, 0, SEEK_END );
        size_t to_read = ftell( in );
        fseek( in, 0, SEEK_SET );

        uint8_t* inbuff = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, to_read );

        if (inbuff && to_read > 0 && to_read == fread( inbuff, 1, to_read, in ))
        {
            uint32_t outputSize = inbuff[0];
            outputSize += inbuff[1] << 8;
            outputSize += inbuff[2] << 16;
            outputSize += inbuff[3] << 24;

            uint8_t* outbuff = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, outputSize );

            if (outbuff)
            {
                losDecode( ctx, inbuff+4, outbuff, &outputSize, to_read-4 );
                fwrite( outbuff, 1, outputSize, out );
                align_free( outbuff );
            }
        }

        if (inbuff != nullptr) align_free( inbuff );

        losDeallocateContext(ctx);
    }
}


int main( int argc, const char** argv )
{
    if (argc != 4)
    {
        printf("lib4zip v0.2 alpha\n"
        "(C) 2022-2024, Julien Perrier-cornet. Free software under GPLv3 Licence.\n"
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
        compressLos(in, out);
    else if (argv[1][0] == 'd')
        decompressLos(in, out);

    printf("%s (%ld) -> %s (%ld) in %.3fs\n",
        argv[2], ftell(in), argv[3], ftell(out),
        double(clock()-start)/CLOCKS_PER_SEC);

    return 0;
}

