
#include <cstdio>
#include <cstdlib>


#include "lzaahe_dict.h"
#include "platform.h"



extern "C" void freeLZAAHEDict(struct LZAAHEDict* dict)
{
    if (dict->symbols != nullptr) align_free( dict->symbols );
    if (dict->symbol_buffer != nullptr) align_free( dict->symbol_buffer );
    if (dict->occurence_list != nullptr) align_free( dict->occurence_list );
    if (dict->first_occ_hash != nullptr) align_free( dict->first_occ_hash );
    align_free( dict );
}

extern "C" struct LZAAHEDict* allocateLZAAHEDict()
{
    struct LZAAHEDict* dict = (struct LZAAHEDict*) align_alloc(256, sizeof(struct LZAAHEDict));

    if (dict != nullptr)
    {
        dict->symbols = nullptr;
        dict->symbol_buffer = nullptr;
        dict->occurence_list = nullptr;
        dict->first_occ_hash = nullptr;

        dict->symbols = (struct LZAAHEDict::LZAAHEDictSymbol*) align_alloc(256, MAX_SYMBOLS*sizeof(struct LZAAHEDict::LZAAHEDictSymbol));
        dict->symbol_buffer = (uint8_t*) align_alloc(256, MAX_SYM_TABLE_LEN*sizeof(uint8_t) );
        dict->occurence_list = (union LZAAHEDict::LZAAHEDictOccurenceListEntry*) align_alloc(256, MAX_SYMBOLS*16*sizeof(union LZAAHEDict::LZAAHEDictOccurenceListEntry));
        dict->first_occ_hash = (struct LZAAHEDict::LZAAHEDictFirstOcc*) align_alloc(256, SYM_HASH_SZ*sizeof(struct LZAAHEDict::LZAAHEDictFirstOcc));

        if (dict->symbols == nullptr || dict->symbol_buffer == nullptr || dict->occurence_list == nullptr || dict->first_occ_hash == nullptr)
        {
            freeLZAAHEDict( dict );
            dict = nullptr;
        }
    }

    return dict;
}

extern "C" void initLZAAHEDict(struct LZAAHEDict* dict)
{
}


extern "C" void gatherOccurences(struct LZAAHEDict* dict, uint8_t* input, uint32_t size)
{
}


extern "C" void writeOccurences(struct LZAAHEDict* dict, struct ArithCtx *arith)
{
}


extern "C" void readOccurences(struct LZAAHEDict* dict, struct ArithCtx *arith, struct LZAAHEDictOccurence* occ, uint32_t *n_occurences, uint32_t max_occurences)
{
}

