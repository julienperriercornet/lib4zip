#pragma once


#include <cstdint>


struct BitIOCtx {
    // I/O to memory
    uint32_t buffer_size, buffer_idx;
    uint8_t* buffer;

    // The next byte to output
    uint32_t bits_in_B;
    uint32_t B;
};


static inline void bitio_init(struct BitIOCtx* ctx, uint8_t* buffer, uint32_t size)
{
    ctx->buffer = buffer;
    ctx->buffer_size = size;
    ctx->buffer_idx = 0;
    ctx->bits_in_B = 0;
}


static uint32_t bitio_getoutptr(struct BitIOCtx* ctx)
{
    if (ctx->bits_in_B)
        return ctx->buffer_idx+4;
    else
        return ctx->buffer_idx;
}


static inline void bitio_putchar(struct BitIOCtx* ctx, uint8_t b )
{
    if (ctx->buffer_idx < ctx->buffer_size) {
        ctx->buffer[ctx->buffer_idx++] = b;
    }
}


static inline void bitio_write(struct BitIOCtx* ctx, const uint32_t bit)
{
    ctx->B = (ctx->B << 1) | bit;
    ctx->bits_in_B++;

    if (ctx->bits_in_B == 32)
    {
        bitio_putchar(ctx, (ctx->B >> 24) & 0xFF);
        bitio_putchar(ctx, (ctx->B >> 16) & 0xFF);
        bitio_putchar(ctx, (ctx->B >> 8) & 0xFF);
        bitio_putchar(ctx, ctx->B);
        ctx->B = 0;
        ctx->bits_in_B = 0;
    }
}


static inline void bitio_write(struct BitIOCtx* ctx, uint32_t n_bits, uint32_t bits)
{
    if ((ctx->bits_in_B + n_bits) <= 32)
    {
        ctx->B = (ctx->B << n_bits) | bits;
        ctx->bits_in_B += n_bits;

        if (ctx->bits_in_B == 32)
        {
            bitio_putchar(ctx, (ctx->B >> 24) & 0xFF);
            bitio_putchar(ctx, (ctx->B >> 16) & 0xFF);
            bitio_putchar(ctx, (ctx->B >> 8) & 0xFF);
            bitio_putchar(ctx, ctx->B);
            ctx->B = 0;
            ctx->bits_in_B = 0;
        }
    }
    else
    {
        uint32_t remain_bits = 32 - ctx->bits_in_B;
        ctx->B = (ctx->B << remain_bits) | ((bits >> (n_bits - remain_bits)) & ((1 << remain_bits) - 1) );
        bitio_putchar(ctx, (ctx->B >> 24) & 0xFF);
        bitio_putchar(ctx, (ctx->B >> 16) & 0xFF);
        bitio_putchar(ctx, (ctx->B >> 8) & 0xFF);
        bitio_putchar(ctx, ctx->B);
        ctx->B = bits & ((1 << (n_bits - remain_bits)) - 1);
        ctx->bits_in_B = n_bits - remain_bits;
    }
}


static inline void bitio_finalize(struct BitIOCtx* ctx)
{
    if (ctx->bits_in_B && ctx->buffer_idx<ctx->buffer_size)
        ctx->buffer[ctx->buffer_idx++] = ctx->B;
}

