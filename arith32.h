#pragma once

/*
** Lib4zip 32 bit arithmetic encoder
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

#include <cstdint>

//#define ARITH_DEBUG

#ifdef ARITH_DEBUG
#include <cassert>
#endif


#define ARITH_PRECISION 30


struct ArithCtx {
    // The current state: interval [a,b]
    uint32_t a, b, x;
    uint32_t pending_bits, bits_in_B;

    // I/O to memory
    uint32_t buffer_size, buffer_idx;
    uint8_t* buffer;

    // The next byte to output
    uint8_t B;
};


static void arith_init(struct ArithCtx* ctx, uint8_t* buffer, uint32_t size)
{
    ctx->a = 0;
    ctx->b = 0xFFFFFFFF;
    ctx->x = 0;

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
    if (ctx->buffer_idx < ctx->buffer_size)
    {
        ctx->buffer[ctx->buffer_idx++] = b;
    }
}


static inline int arith_bit_read(struct ArithCtx* ctx)
{
    if (ctx->bits_in_B == 0)
    {
        ctx->B = arith_getchar(ctx);
        ctx->bits_in_B = 8;
    }

    ctx->bits_in_B--; // 7..0
    return (ctx->B >> ctx->bits_in_B) & 1;
}


static inline void arith_bit_write(struct ArithCtx* ctx, const int bit)
{
    ctx->B = (ctx->B << 1) | bit;
    ctx->bits_in_B++;

    if (ctx->bits_in_B == 8)
    {
        arith_putchar(ctx, ctx->B);
        ctx->B = 0;
        ctx->bits_in_B = 0;
    }
}


static inline void arith_bit_write_with_pending(struct ArithCtx* ctx, const int bit)
{
    arith_bit_write(ctx, bit);
    for (; ctx->pending_bits > 0; ctx->pending_bits--)
        arith_bit_write(ctx, bit ^ 1);
}


static void arith_finalize(struct ArithCtx* ctx)
{
    do
    {
        arith_bit_write_with_pending(ctx, ctx->a >> 31); // output pending byte from a
        ctx->a <<= 1;
    }
    while (ctx->bits_in_B != 0);
}


static void arith_prefetch(struct ArithCtx* ctx)
{
    for (int i = 0; i < 32; ++i)
        ctx->x = (ctx->x << 1) | arith_bit_read(ctx);
}


static void arith_encodebit(struct ArithCtx* ctx, uint32_t p, const int bit)
{
    if (p == 0) return;

#ifdef ARITH_DEBUG
    assert(p > 0 && p < (1u << ARITH_PRECISION));
#endif

    uint32_t xmid = ctx->a + uint32_t((uint64_t(ctx->b - ctx->a) * p) >> ARITH_PRECISION);

#ifdef ARITH_DEBUG
    assert(xmid >= ctx->a && xmid < ctx->b);
#endif

    bit !=0 ? (ctx->b = xmid) : (ctx->a = xmid + 1);

    while (((ctx->a ^ ctx->b) >> 31) == 0) {  // pass equal leading bits of range
        arith_bit_write_with_pending(ctx, ctx->b >> 31);
        ctx->a <<= 1;
        ctx->b = (ctx->b << 1) | 1;
    }
    while (ctx->a >= 0x40000000 && ctx->b < 0xC0000000) {
        ctx->pending_bits++;
        ctx->a = (ctx->a << 1) & 0x7FFFFFFF;
        ctx->b = (ctx->b << 1) | 0x80000001;
    }
}


static int arith_decodebit(struct ArithCtx* ctx, uint32_t p)
{
    if (p == 0) return 0;

#ifdef ARITH_DEBUG
    assert(p > 0 && p < (1u<< ARITH_PRECISION));
#endif

    uint32_t xmid = ctx->a + uint32_t((uint64_t(ctx->b - ctx->a) * p) >> ARITH_PRECISION);

#ifdef ARITH_DEBUG
    assert(xmid >= ctx->a && xmid < ctx->b);
#endif

    int bit = ctx->x <= xmid;
    bit != 0 ? (ctx->b = xmid) : (ctx->a = xmid + 1);

    while (((ctx->a ^ ctx->b) >> 31) == 0) {  // pass equal leading bits of range
        ctx->a <<= 1;
        ctx->b = (ctx->b << 1) | 1;
        ctx->x = (ctx->x << 1) | arith_bit_read( ctx );
    }
    while (ctx->a >= 0x40000000 && ctx->b < 0xC0000000) {
        ctx->a = (ctx->a << 1) & 0x7FFFFFFF;
        ctx->b = (ctx->b << 1) | 0x80000001;
        ctx->x = (ctx->x << 1) ^ 0x80000000;
        ctx->x += arith_bit_read( ctx );
    }

    return bit;
}
