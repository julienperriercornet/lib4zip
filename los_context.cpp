/*
** Lib4zip line of sight (los) codec context implementation.
** Copyright (C) 2024 Julien Perrier-cornet
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
#include <cstring>


#include "los_context.h"


extern "C" void losDeallocateContext(struct LOSCompressionContext* ctx)
{
    if (ctx->presence) align_free(ctx->presence);
    if (ctx->dictidx) align_free(ctx->dictidx);
    if (ctx->dict) align_free(ctx->dict);
    if (ctx->arith) align_free(ctx->arith);
    align_free(ctx);
}


extern "C" struct LOSCompressionContext* losAllocateContext()
{
    struct LOSCompressionContext* context = (struct LOSCompressionContext*) align_alloc( MAX_CACHE_LINE_SIZE, sizeof(struct LOSCompressionContext) );

    if (context)
    {
        context->presence = nullptr;
        context->dictidx = nullptr;
        context->dict = nullptr;
        context->arith = nullptr;

        context->presence = (uint32_t*) align_alloc( MAX_CACHE_LINE_SIZE, 64*(1<<24) );
        context->dictidx = (uint32_t*) align_alloc( MAX_CACHE_LINE_SIZE, sizeof(uint32_t)*(1<<24) );
        context->dict = (uint8_t*) align_alloc( MAX_CACHE_LINE_SIZE, 64*(1<<24) );
        context->arith = (struct ArithCtx*) align_alloc( MAX_CACHE_LINE_SIZE, sizeof(struct ArithCtx) );

        if (!context->presence || !context->dictidx || !context->dict || !context->arith)
        {
            losDeallocateContext(context);
            context = nullptr;
        }
    }

    return context;
}

