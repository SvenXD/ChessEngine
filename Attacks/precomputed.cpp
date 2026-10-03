#include "precomputed.h"

Bitboard pawn_attacks[2][64];
Bitboard knight_attacks[64];
Bitboard king_attacks[64];

/* Look up table for O(1) search on attacked squares in n square

    Since knights, kings and pawns will ALWAYS attack the SAME squares
    we can pre store them WHERE they would attack at N location regardless of pieces
    
*/

// ---------------------------------------------------------------------------
// Knights: 8 possible (rank, file) offsets, bounds-checked.
// ---------------------------------------------------------------------------
static void init_knight_attacks() {
    static const int dr[8] = { 2,  2, -2, -2,  1,  1, -1, -1}; // change in rank (Row)
    static const int df[8] = { 1, -1,  1, -1,  2, -2,  2, -2}; // change in file (column)

    for (int s = 0; s < 64; ++s) {
        int r = rank_of(s);
        int f = file_of(s);
        Bitboard b = 0;

        for (int i = 0; i < 8; ++i) {
            int nr = r + dr[i];
            int nf = f + df[i];
            if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8)
                set_bit(b, make_square(nf, nr));
        }

        knight_attacks[s] = b;
    }
}

// ---------------------------------------------------------------------------
// Kings: 8 one-step offsets, bounds-checked.
// ---------------------------------------------------------------------------
static void init_king_attacks() {
    static const int dr[8] = { 1,  1,  1,  0,  0, -1, -1, -1};
    static const int df[8] = {-1,  0,  1, -1,  1, -1,  0,  1};

    for (int s = 0; s < 64; ++s) {
        int r = rank_of(s);
        int f = file_of(s);
        Bitboard b = 0;

        for (int i = 0; i < 8; ++i) {
            int nr = r + dr[i];
            int nf = f + df[i];
            if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8)
                set_bit(b, make_square(nf, nr));
        }

        king_attacks[s] = b;
    }
}

// ---------------------------------------------------------------------------
// Pawns: white attacks up-left / up-right (rank+1).
//        black attacks down-left / down-right (rank-1).
// File bounds are checked so a2 does NOT attack h3 (and h2 does not attack a3).
// ---------------------------------------------------------------------------
static void init_pawn_attacks() {
    for (int s = 0; s < 64; ++s) {
        int r = rank_of(s);
        int f = file_of(s);
        Bitboard white = 0;
        Bitboard black = 0;

        // White pawn pushes "up" the board (toward rank 8).
        if (r < 7) {
            if (f > 0) set_bit(white, make_square(f - 1, r + 1)); // up-left
            if (f < 7) set_bit(white, make_square(f + 1, r + 1)); // up-right
        }

        // Black pawn pushes "down" the board (toward rank 1).
        if (r > 0) {
            if (f > 0) set_bit(black, make_square(f - 1, r - 1)); // down-left
            if (f < 7) set_bit(black, make_square(f + 1, r - 1)); // down-right
        }

        pawn_attacks[WHITE][s] = white;
        pawn_attacks[BLACK][s] = black;
    }
}

// ---------------------------------------------------------------------------
void init_attack_tables() {
    init_knight_attacks();
    init_king_attacks();
    init_pawn_attacks();
}