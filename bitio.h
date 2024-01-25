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
    ctx->B = 0;
}


static inline uint32_t bitio_getoutptr(struct BitIOCtx* ctx)
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


static inline uint8_t bitio_getchar( struct BitIOCtx* ctx )
{
    if (ctx->buffer_idx < ctx->buffer_size) {
        return ctx->buffer[ctx->buffer_idx++];
    }
    return 0;
}


static inline void bitio_flush(struct BitIOCtx* ctx)
{
    bitio_putchar(ctx, (ctx->B >> 24) & 0xFF);
    bitio_putchar(ctx, (ctx->B >> 16) & 0xFF);
    bitio_putchar(ctx, (ctx->B >> 8) & 0xFF);
    bitio_putchar(ctx, ctx->B & 0xFF);
    ctx->bits_in_B = 0;
}


static inline void bitio_finalize(struct BitIOCtx* ctx)
{
    if (ctx->bits_in_B != 0)
    {
        if (ctx->bits_in_B != 32) ctx->B <<= (32 - ctx->bits_in_B);
        bitio_flush(ctx);
    }
}


static inline void bitio_write_bit(struct BitIOCtx* ctx, const uint32_t bit)
{
    ctx->B = (ctx->B << 1) | bit;
    ctx->bits_in_B++;

    if (ctx->bits_in_B == 32)
    {
        bitio_flush(ctx);
        ctx->B = 0;
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
            bitio_flush(ctx);
            ctx->B = 0;
        }
    }
    else
    {
        uint32_t remain_bits = 32 - ctx->bits_in_B;
        ctx->B = (ctx->B << remain_bits) | ((bits >> (n_bits - remain_bits)) & ((1 << remain_bits) - 1) );
        bitio_flush(ctx);
        ctx->B = bits & ((1 << (n_bits - remain_bits)) - 1);
        ctx->bits_in_B = n_bits - remain_bits;
    }
}


static inline void bitio_prefetch(struct BitIOCtx* ctx)
{
    ctx->B = bitio_getchar(ctx);
    ctx->B = (ctx->B << 8) | bitio_getchar(ctx);
    ctx->B = (ctx->B << 8) | bitio_getchar(ctx);
    ctx->B = (ctx->B << 8) | bitio_getchar(ctx);
    ctx->bits_in_B = 32;
}


static inline uint32_t bitio_read_bit(struct BitIOCtx* ctx)
{
    ctx->bits_in_B--;

    uint32_t d = (ctx->B >> ctx->bits_in_B) & 1;

    if (ctx->bits_in_B == 0)
        bitio_prefetch( ctx );

    return d;
}


static inline uint32_t bitio_read(struct BitIOCtx* ctx, uint32_t n_bits)
{
    uint32_t b = n_bits < ctx->bits_in_B ? n_bits : ctx->bits_in_B;

    ctx->bits_in_B -= b;
    n_bits -= b;

    uint32_t d = (ctx->B >> ctx->bits_in_B) & ((1 << b) - 1);

    if (ctx->bits_in_B == 0)
        bitio_prefetch( ctx );

    if (n_bits > 0)
    {
        ctx->bits_in_B -= n_bits;
        d = (d << n_bits) | ((ctx->B >> ctx->bits_in_B) & ((1 << n_bits) - 1));
    }

    return d;
}

