#pragma once 
#include "types.h"

/* Chess board util will have the 64 squares as bits of either there is a peuce in tehre or not,
when placing bits it will change a new unordered long long values and then jsut or with the original board*/

/* 1ull starts with a 1, 0ull starts with 0*/

inline void     set_bit(Bitboard& b, Square s)    { b |=  (1ULL << s); }
inline void     clear_bit(Bitboard& b, Square s)  { b &= ~(1ULL << s); }
inline bool     test_bit(Bitboard b, Square s)    { return b & (1ULL << s); }


inline int      popcount(Bitboard b)              { return __builtin_popcountll(b); }
inline Square   lsb(Bitboard b)                   { return __builtin_ctzll(b); }
inline Square   pop_lsb(Bitboard& b)              { Square s = lsb(b); b &= b-1; return s; }
