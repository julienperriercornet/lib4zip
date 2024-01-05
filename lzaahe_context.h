#pragma once


#include "lib4zip.h"
#include "arith.h"
#include "platform.h"



#define LZAAHE_BYTEHASH_SZ (256*32)
#define LZAAHE_RINGBUFFER_BITS (12)
#define LZAAHE_RINGBUFFER_SZ (1<<LZAAHE_RINGBUFFER_BITS)
#define LZAAHE_REFHASH_BITS (20)
#define LZAAHE_REFHASH_SZ (1<<LZAAHE_REFHASH_BITS)

