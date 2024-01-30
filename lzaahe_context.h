#pragma once


#include "lib4zip.h"
#include "bitio.h"
#include "platform.h"



#define LZAAHE_REFHASH_BITS (LZAAHE_BLOCK_BITS)
#define LZAAHE_REFHASH_SZ (1<<LZAAHE_REFHASH_BITS)
#define LZAAHE_REFHASH_ENTITIES (2)
#define LZAAHE_MAX_SYMBOLS (1<<(LZAAHE_BLOCK_BITS-3))

