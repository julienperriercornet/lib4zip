#pragma once


#include <cstdint>


#define MAX_SYMBOLS 65536
#define MAX_SYM_TABLE_LEN 65536*16
#define SYM_HASH_SZ ((1<<22)+16)


struct LZAAHEDict {

    struct LZAAHEDictSymbol {
        uint32_t start, len;
    };

    struct LZAAHEDictSymbol* symbols;
    uint32_t n_symbols;

    uint8_t* symbol_buffer;
    uint32_t symbol_buffer_pos;

    union LZAAHEDictOccurenceListEntry {
        struct first {
            uint32_t type; // Type of this cell. This enables iterating through the buffer of occurences.
            uint32_t last_idx; // Last index of a linked list node to insert occurences quickly
            uint32_t longest_match1;
            uint32_t longest_match2; // The 2 strings start index which matched the longest with each other
            uint32_t longest_match_len; // Longest match length
            uint32_t occurences[2];
            uint32_t next_idx; // Next index of a node within the linked list (-1 is the null element)
        };
        struct follow {
            uint32_t type;
            uint32_t occurences[6];
            uint32_t next_idx; // Next index of a node within the linked list (-1 is the null element)
        };
    };

    union LZAAHEDictOccurenceListEntry *occurence_list;
    uint32_t last_occurence_pos;

    struct LZAAHEDictFirstOcc {
        uint32_t sym, pos;
    };

    struct LZAAHEDictFirstOcc* first_occ_hash;

};


struct LZAAHEDictOccurence {
    uint32_t pos, symbol, matchlen;
};


extern "C" struct LZAAHEDict* allocateLZAAHEDict();
extern "C" void freeLZAAHEDict(struct LZAAHEDict* dict);
extern "C" void initLZAAHEDict(struct LZAAHEDict* dict);
extern "C" void gatherOccurences(struct LZAAHEDict* dict, uint8_t* input, uint32_t size);
extern "C" void writeOccurences(struct LZAAHEDict* dict, struct ArithCtx *arith);
extern "C" void readOccurences(struct LZAAHEDict* dict, struct ArithCtx *arith, struct LZAAHEDictOccurence* occ, uint32_t *n_occurences, uint32_t max_occurences);

