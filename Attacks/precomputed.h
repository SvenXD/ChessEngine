#pragma once
#include "../Utils/types.h"
#include "../Utils/bit_utils.h"
/* Extern to define their existence before filling them
    also because of the ODR (One definition rule) if used in multiples files
    withut extern or inline, it chockes and dies bc multiple definitions are in there
    so we precompute stating it exists here and everyone shares teh same point here

    So tldr: similar to javas StatiC
*/
extern Bitboard pawn_attacks[2][64]; // white and black in that order
extern Bitboard knight_attacks[64];
extern Bitboard king_attacks[64];

void init_attack_tables();