/*
** Lib4zip lzaahe dictionnary implementation.
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
#include <cstring>
#include <cassert>


#include "lzaahe_dict.h"
#include "platform.h"



extern "C" void freeLZAAHEDict(struct LZAAHEDict* dict)
{
    if (dict->symbols != nullptr) align_free( dict->symbols );
    if (dict->symbol_buffer != nullptr) align_free( dict->symbol_buffer );
    if (dict->occurence_list != nullptr) align_free( dict->occurence_list );
    if (dict->first_occ_hash != nullptr) align_free( dict->first_occ_hash );
    if (dict->hit_occ_hash != nullptr) align_free( dict->hit_occ_hash );
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
        dict->occurence_list = (union LZAAHEDict::LZAAHEDictOccurenceListEntry*) align_alloc(256, MAX_SYMBOLS*32*sizeof(union LZAAHEDict::LZAAHEDictOccurenceListEntry));
        dict->first_occ_hash = (struct LZAAHEDict::LZAAHEDictHashOcc*) align_alloc(256, SYM_HASH_SZ*sizeof(struct LZAAHEDict::LZAAHEDictHashOcc));
        dict->hit_occ_hash = (struct LZAAHEDict::LZAAHEDictHashOcc*) align_alloc(256, SYM_HASH_SZ*sizeof(struct LZAAHEDict::LZAAHEDictHashOcc));

        if (dict->symbols == nullptr || dict->symbol_buffer == nullptr || dict->occurence_list == nullptr ||
            dict->first_occ_hash == nullptr || dict->hit_occ_hash == nullptr)
        {
            freeLZAAHEDict( dict );
            dict = nullptr;
        }
    }

    return dict;
}


extern "C" void initLZAAHEDict(struct LZAAHEDict* dict)
{
    dict->n_symbols = 0;
    dict->symbol_buffer_pos = 0;
    dict->last_occurence_pos = 0;

    memset( dict->first_occ_hash, -1, SYM_HASH_SZ*sizeof(struct LZAAHEDict::LZAAHEDictHashOcc) );
    memset( dict->hit_occ_hash, -1, SYM_HASH_SZ*sizeof(struct LZAAHEDict::LZAAHEDictHashOcc) );
    memset( dict->occurence_list, -1, MAX_SYMBOLS*32*sizeof(union LZAAHEDict::LZAAHEDictOccurenceListEntry) );
}


static inline uint32_t getHash( uint32_t h )
{
    return ((h >> 19) ^ (h & 0x7FFFF)) << 1;
}


static inline uint32_t getHashOcc(uint32_t which, uint32_t pos, struct LZAAHEDict::LZAAHEDictHashOcc* hash)
{
    uint32_t firstpos = -1;
    uint32_t i = getHash(which);

    while (i < SYM_HASH_SZ && hash[i].pos != -1 && hash[i].sym != which) i++;

    if (i < SYM_HASH_SZ)
    {
        if (hash[i].sym == which)
        {
            firstpos = hash[i].pos;
        }
        else
        {
            hash[i].pos = pos;
            hash[i].sym = which;
        }
    }

    return firstpos;
}


static inline uint32_t getHashOccCurr(uint32_t which, uint32_t pos, struct LZAAHEDict::LZAAHEDictHashOcc* hash)
{
    uint32_t firstpos = -1;
    uint32_t i = getHash(which);

    while (i < SYM_HASH_SZ && hash[i].pos != -1 && hash[i].sym != which) i++;

    if (i < SYM_HASH_SZ)
    {
        if (hash[i].sym == which)
        {
            firstpos = hash[i].pos;
        }
        else
        {
            hash[i].pos = pos;
            hash[i].sym = which;
            firstpos = pos;
        }
    }

    return firstpos;
}


static inline uint32_t matchlen( uint8_t *inbuff, uint32_t first, uint32_t second, uint32_t size )
{
    uint32_t i = 4; // The first 4 bytes of both strings are already matching at this point

    while ((first+i) < size && (second+i) < size && i < 260 && inbuff[first+i] == inbuff[second+i]) i++;

    return i;
}


static inline bool insertOccurence( union LZAAHEDict::LZAAHEDictOccurenceListEntry *first_entry, union LZAAHEDict::LZAAHEDictOccurenceListEntry *last_entry, uint8_t* buffer, uint32_t buffer_size, uint32_t pos, uint32_t &len )
{
    bool caninsert = false;

    if (last_entry->f.type == 2)
    {
        uint32_t i=0;

        while (i<6 && last_entry->o.occurences[i] != -1) i++;

        if (i != 6)
        {
            uint32_t m1 = matchlen( buffer, first_entry->f.longest_match1, pos, buffer_size );
            uint32_t m2 = matchlen( buffer, first_entry->f.longest_match2, pos, buffer_size );

            if (m1 > m2 && m1 > first_entry->f.longest_match_len)
            {
                first_entry->f.longest_match2 = pos;
                first_entry->f.longest_match_len = m1;
            }

            if (m2 > first_entry->f.longest_match_len)
            {
                first_entry->f.longest_match1 = pos;
                first_entry->f.longest_match_len = m2;
            }

            len = m1 > m2 ? m1 : m2;

            last_entry->o.occurences[i] = pos;

            caninsert = true;
        }
    }

    return caninsert;
}


static uint32_t addHit( struct LZAAHEDict* dict, uint8_t* buffer, uint32_t buffer_size, uint32_t which, uint32_t firstocc, uint32_t occ )
{
    uint32_t i = getHashOccCurr( which, dict->last_occurence_pos, dict->hit_occ_hash );
    uint32_t len = 4;

    if (i != -1)
    {
        if (i == dict->last_occurence_pos)
        {
            if (dict->last_occurence_pos < MAX_SYMBOLS*32)
            {
                dict->occurence_list[i].f.type = 1;
                dict->occurence_list[i].f.last_idx = i;
                dict->occurence_list[i].f.longest_match1 = firstocc;
                dict->occurence_list[i].f.longest_match2 = occ;
                dict->occurence_list[i].f.longest_match_len = len = matchlen( buffer, firstocc, occ, buffer_size );
                dict->occurence_list[i].f.occurences[0] = firstocc;
                dict->occurence_list[i].f.occurences[1] = occ;

                dict->last_occurence_pos++;
            }
        }
        else
        {
            if (!insertOccurence( dict->occurence_list+i, dict->occurence_list+dict->occurence_list[i].f.last_idx, buffer, buffer_size, occ, len ))
            {
                if (dict->last_occurence_pos < MAX_SYMBOLS*32)
                {
                    dict->occurence_list[dict->occurence_list[i].f.last_idx].f.next_idx = dict->last_occurence_pos;
                    dict->occurence_list[i].f.last_idx = dict->last_occurence_pos;

                    dict->occurence_list[dict->last_occurence_pos].o.type = 2;
                    dict->occurence_list[dict->last_occurence_pos].o.occurences[0] = occ;

                    uint32_t m1 = matchlen( buffer, dict->occurence_list[i].f.longest_match1, occ, buffer_size );
                    uint32_t m2 = matchlen( buffer, dict->occurence_list[i].f.longest_match2, occ, buffer_size );

                    if (m1 > m2 && m1 > dict->occurence_list[i].f.longest_match_len)
                    {
                        dict->occurence_list[i].f.longest_match2 = occ;
                        dict->occurence_list[i].f.longest_match_len = m1;
                    }

                    if (m2 > dict->occurence_list[i].f.longest_match_len)
                    {
                        dict->occurence_list[i].f.longest_match1 = occ;
                        dict->occurence_list[i].f.longest_match_len = m2;
                    }

                    len = m1 > m2 ? m1 : m2;

                    dict->last_occurence_pos++;
                }
            }
        }
    }

    return len;
}


static uint32_t symbolGain( union LZAAHEDict::LZAAHEDictOccurenceListEntry *occurences, uint32_t symbol, uint8_t* buffer, uint32_t buffer_size )
{
    uint32_t gain = 0;
    uint32_t longest1 = occurences[symbol].f.longest_match1;
    uint32_t longest2 = occurences[symbol].f.longest_match2;
    uint32_t longest_len = occurences[symbol].f.longest_match_len;
    uint32_t n_occurences = 0;

    do
    {
        if (occurences[symbol].f.type == 1)
        {
            for (uint32_t i=0; i<2; i++)
            {
                uint32_t len;
                if (longest1 != occurences[symbol].f.occurences[i])
                    len = matchlen( buffer, longest1, occurences[symbol].f.occurences[i], buffer_size );
                else
                    len = matchlen( buffer, longest2, occurences[symbol].f.occurences[i], buffer_size );
                if (i>0 && len >= 4) gain += len;
            }

            n_occurences += 2;
        }
        else if (occurences[symbol].f.type == 2)
        {
            for (uint32_t i=0; i<6; i++)
            {
                if (occurences[symbol].o.occurences[i] != -1)
                {
                    uint32_t len;
                    if (longest1 != occurences[symbol].o.occurences[i])
                        len = matchlen( buffer, longest1, occurences[symbol].o.occurences[i], buffer_size );
                    else
                        len = matchlen( buffer, longest2, occurences[symbol].o.occurences[i], buffer_size );
                    if (len >= 4) gain += len;
                    n_occurences ++;
                }
            }
        }
    }
    while ((symbol = occurences[symbol].f.next_idx) != -1) ;

    if ((gain - (n_occurences - 1) * 3) >= 0) return gain - (n_occurences - 1) * 3;
    else return 0;
}


extern "C" void lzaaheGatherOccurences(struct LZAAHEDict* dict, uint8_t* input, uint32_t size)
{
    for (uint32_t i=0; i<size-3; i++)
    {
        uint32_t w = *((uint32_t*) (input+i));
        uint32_t firstocc = getHashOcc( w, i, dict->first_occ_hash );

        if (firstocc != -1)
        {
            i += addHit( dict, input, size, w, firstocc, i ) - 1;
        }
    }

#if 1
    // Print occurences to the console (debug)
    uint32_t count_1=0, count_2=0, sum_gain=0;

    for (uint32_t i=0; i<dict->last_occurence_pos; i++)
    {
        if (dict->occurence_list[i].f.type == 1)
        {
            uint32_t gain = symbolGain(dict->occurence_list, i, input, size);
            //printf( "occurence %d symbolGain: %d\n", i, gain );
            count_1++;
            sum_gain += gain;
        }
        else if (dict->occurence_list[i].f.type == 2)
        {
            count_2++;
        }
    }

    printf( "count_1 %d sum_gain: %d\n", count_1, sum_gain );
#endif
}


extern "C" void lzaaheWriteOccurences(struct LZAAHEDict* dict, struct ArithCtx *arith)
{
}


extern "C" void lzaaheReadOccurences(struct LZAAHEDict* dict, struct ArithCtx *arith, struct LZAAHEDictOccurence* occ, uint32_t *n_occurences, uint32_t max_occurences)
{
}

