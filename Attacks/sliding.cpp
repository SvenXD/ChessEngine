#include "sliding.h"

// Walks outward from `from` in 4 directions, one bit at a time,
// stopping after the first occupied square it hits. Each direction
// is (rank_delta, file_delta). Returns a bitboard of every square
// touched, including the blocker that stopped it.

/* tldr: since they can ge blocked and not have all squares its just a ray to check to where they cango
    The structure it follows is creating a delta rows and delta files to determine which direction would move
    each piece, and since each one has 4 ways of moving (directions) they can be stored as an array of 4
*/

/* Standar util for any attack*/
static Bitboard ray_attacks(Square from, Bitboard occupied,
                            const int dr[4], const int df[4]){

    /* Standar utils getting the rank and files starter to chekc from there*/
    Bitboard attacks = 0;
    int r0 = rank_of(from);
    int f0 = file_of(from);

    /* For cycle to go throigh the delta implaced to check*/
    for (int d = 0; d < 4; ++d) {
        /* Apply deltas */
        int r = r0 + dr[d];
        int f = f0 + df[d];

        /* For cycle to check IF a square it transverses is blocked or is beyond the board
           we store the current square and checks if its busy or not, then set it as an abvailable one
           for the attacks
        */
        while (r >= 0 && r < 8 && f >= 0 && f < 8) {
            Square s = make_square(f, r);
            set_bit(attacks, s);

            // Stop AFTER adding the blocker: you may capture it,
            // but you cannot see through it.
            if (test_bit(occupied, s)) break;

            r += dr[d];
            f += df[d];
        }
    }
    return attacks;
}

/* NOTE abotu STATIC, each one needs to be static the delta so that
    its allocated ONCE in memmory and not REALLOCATED each time the function is called
    which will be called, a TON
*/

Bitboard rook_attacks(Square from, Bitboard occupied) {
    static const int dr[4] = { 1, -1,  0,  0};
    static const int df[4] = { 0,  0,  1, -1};
    return ray_attacks(from, occupied, dr, df);
}

Bitboard bishop_attacks(Square from, Bitboard occupied) {
    static const int dr[4] = { 1,  1, -1, -1};
    static const int df[4] = { 1, -1,  1, -1};
    return ray_attacks(from, occupied, dr, df);
}


/* Since queen is practically the rook and bishop, its jsut combining them with the OR*/
Bitboard queen_attacks(Square from, Bitboard occupied) {
    return rook_attacks(from, occupied) | bishop_attacks(from, occupied);
}