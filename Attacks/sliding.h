#pragma once
#include "../Utils/types.h"
#include "../Utils/bit_utils.h"

/* We need the Square they are exactly at, and the board containing the locations
    of all the pieces currently on it
*/
Bitboard rook_attacks  (Square from, Bitboard occupied);
Bitboard bishop_attacks(Square from, Bitboard occupied);
Bitboard queen_attacks (Square from, Bitboard occupied);