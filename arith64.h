#pragma once


#include <cstdint>


//#define ARITH_DEBUG

#ifdef ARITH_DEBUG
#include <cassert>
#endif


#define ARITH_PRECISION 30


struct ArithCtx {
    // The current state: interval [a,b]
    uint64_t a, b, x;
    uint32_t pending_bits, bits_in_B;

    // I/O to memory
    uint32_t buffer_size, buffer_idx;
    uint8_t* buffer;

    // The next byte to output
    uint8_t B;
};


static void arith_init(struct ArithCtx* ctx, uint8_t* buffer, uint32_t size)
{
    ctx->a = 0x0000000000000000ULL;
    ctx->b = 0xFFFFFFFFFFFFFFFFULL;
    ctx->x = 0x0000000000000000ULL;

    ctx->pending_bits = 0;
    ctx->bits_in_B = 0;

    ctx->buffer = buffer;
    ctx->buffer_size = size;
    ctx->buffer_idx = 0;
}


static uint32_t arith_getoutptr(struct ArithCtx* ctx)
{
    if (ctx->bits_in_B)
        return ctx->buffer_idx+1;
    else
        return ctx->buffer_idx;
}


static inline int arith_getchar(struct ArithCtx* ctx)
{
    if (ctx->buffer_idx < ctx->buffer_size)
    {
        return ctx->buffer[ctx->buffer_idx++];
    }
    return -1;
}


static inline void arith_putchar(struct ArithCtx* ctx, uint8_t b )
{
    if (ctx->buffer_idx < ctx->buffer_size) {
        ctx->buffer[ctx->buffer_idx++] = b;
    }
}


static inline int arith_bit_read(struct ArithCtx* ctx) {
    if (ctx->bits_in_B == 0) {
        ctx->B = arith_getchar(ctx); // EOF is OK
        ctx->bits_in_B = 8;
    }
    ctx->bits_in_B--; //7..0
    return (ctx->B >> ctx->bits_in_B) & 1;
}


static inline void arith_bit_write(struct ArithCtx* ctx, const int bit) {
    ctx->B = (ctx->B << 1) | bit;
    ctx->bits_in_B++;
    if (ctx->bits_in_B == 8) {
        arith_putchar(ctx, ctx->B);
        ctx->B = 0;
        ctx->bits_in_B = 0;
    }
}


static inline void arith_bit_write_with_pending(struct ArithCtx* ctx, const int bit) {
    arith_bit_write(ctx, bit);
    for (; ctx->pending_bits > 0; ctx->pending_bits--)
        arith_bit_write(ctx, bit ^ 1);
}


static void arith_finalize(struct ArithCtx* ctx) {
    do {
        arith_bit_write_with_pending(ctx, ctx->a >> 63); // pad pending byte from x1
        ctx->a <<= 1;
    } while (ctx->bits_in_B != 0);
}


static void arith_prefetch(struct ArithCtx* ctx) {
    for (int i = 0; i < 64; ++i)
        ctx->x = (ctx->x << 1) | arith_bit_read(ctx);
}


static inline uint64_t arith_madd_shift( uint64_t a, uint32_t b, uint64_t c )
{
    uint64_t low = (a & 0xFFFFFFFFULL) * b;
    uint64_t high = (a >> 32) * b;

    return (low >> ARITH_PRECISION) + (high << (32 - ARITH_PRECISION)) + c;
}


static void arith_encodebit(struct ArithCtx* ctx, uint32_t p, const int bit)
{
    if (p == 0) return;

#ifdef ARITH_DEBUG
    assert(p > 0 && p < (1u << ARITH_PRECISION));
#endif

    uint64_t xmid = arith_madd_shift( ctx->b - ctx->a, p, ctx->a );

#ifdef ARITH_DEBUG
    assert(xmid >= ctx->a && xmid < ctx->b);
#endif

    bit != 0 ? (ctx->b = xmid) : (ctx->a = xmid + 1);

    while (((ctx->a ^ ctx->b) >> 63) == 0) {  // pass equal leading bits of range
        arith_bit_write_with_pending(ctx, ctx->b >> 63);
        ctx->a <<= 1;
        ctx->b = (ctx->b << 1) | 1;
    }
    while (ctx->a >= 0x4000000000000000ULL && ctx->b < 0xC000000000000000ULL) {
        ctx->pending_bits++;
        ctx->a = (ctx->a << 1) & 0x7FFFFFFFFFFFFFFFULL;
        ctx->b = (ctx->b << 1) | 0x8000000000000001ULL;
    }
}


static int arith_decodebit(struct ArithCtx* ctx, uint32_t p)
{
    if (p == 0) return 0;

#ifdef ARITH_DEBUG
    assert(p > 0 && p < (1u<< ARITH_PRECISION));
#endif

    uint64_t xmid = arith_madd_shift( ctx->b - ctx->a, p, ctx->a );

#ifdef ARITH_DEBUG
    assert(xmid >= ctx->a && xmid < ctx->b);
#endif

    int bit = ctx->x <= xmid;
    bit != 0 ? (ctx->b = xmid) : (ctx->a = xmid + 1);

    while (((ctx->a ^ ctx->b) >> 63) == 0) {  // pass equal leading bits of range
        ctx->a <<= 1;
        ctx->b = (ctx->b << 1) | 1;
        ctx->x = (ctx->x << 1) | arith_bit_read( ctx );
    }
    while (ctx->a >= 0x4000000000000000ULL && ctx->b < 0xC000000000000000ULL) {
        ctx->a = (ctx->a << 1) & 0x7FFFFFFFFFFFFFFFULL;
        ctx->b = (ctx->b << 1) | 0x8000000000000001ULL;
        ctx->x = (ctx->x << 1) ^ 0x8000000000000000ULL;
        ctx->x += arith_bit_read( ctx );
    }
    return bit;
}
