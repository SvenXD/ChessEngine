#pragma once 
#include "types.h"

/* Chess board util will have the 64 squares as bits of either there is a peuce in tehre or not,
when placing bits it will change a new unordered long long values and then jsut or with the original board*/

/* 1ull starts with a 1, 0ull starts with 0*/

// sets by starting 000..1 and moving them s being the square
inline void     set_bit(Bitboard& b, Square s)    { b |=  (1ULL << s); }

// sets by starting in 1111..0 and moving it and then ANDing it
inline void     clear_bit(Bitboard& b, Square s)  { b &= ~(1ULL << s); }
inline bool     test_bit(Bitboard b, Square s)    { return b & (1ULL << s); }

//Population count, says how many bits are 1 LL to be LKL
inline int      popcount(Bitboard b)              { return __builtin_popcountll(b); }

//least significant bit Returns the number of 0z behing the first 1, ej  10010 = 1 , 11100 = 2
inline Square   lsb(Bitboard b)                   { return __builtin_ctzll(b); }

// Checks the first bit it finds and turns it op, useful for small things and for cycles where you
// want to pop items

/*
It's mostly to check if something or a square has something, and not just because of the priority of the smallest one 
but because we can use it as a better checker instead of a loop with << 1
*/
inline Square   pop_lsb(Bitboard& b)              { Square s = lsb(b); b &= b-1; return s; }

