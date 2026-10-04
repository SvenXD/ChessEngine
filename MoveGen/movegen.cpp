#include "movegen.h"
#include "../Attacks/precomputed.h"
#include "../Attacks/sliding.h"

// ---------------------------------------------------------------------------
// Shared helper: turn a bitboard of destinations into Move objects.
//   from    = the square the piece is on
//   attacks = squares this piece can reach (already masked to legal targets)
//   enemy   = enemy-occupied squares, used to flag captures
// ---------------------------------------------------------------------------
static void emit_moves(Square from, Bitboard attacks, Bitboard enemy,
                       std::vector<Move>& out)
{
    while (attacks) {
        // Pops the copied attacks
        Square to = pop_lsb(attacks);
        //Checks for the move info if there is a peice or not to change the flag
        MoveFlag flag = test_bit(enemy, to) ? FLAG_CAPTURE : FLAG_QUIET;
        out.push_back(make_move(from, to, flag));
    }
}


/* Generates all the current possible knight moves taking into consideration all that are in the board*/
static void generate_knight_moves(const Position& pos, std::vector<Move>& out){

    /* Important to get the color or each and the type of piece*/
    Color us    = pos.side_to_move;
    Color them  = static_cast<Color>(1 - us);
    Piece our_knight = (us == WHITE) ? W_KNIGHT : B_KNIGHT;

    Bitboard knights = pos.pieces[our_knight];
    Bitboard targets = ~pos.colors[us];   // empty or enemy squares
    Bitboard enemy   = pos.colors[them];

    /* Knights regresara las posiciones donde se ubicaran todos los caballos de nuestro color

       Targets regresara TODAS las casillas que esten vacias o con piezas opuestas, seran posibles
       puntos de ataque

        Enemy will return the positions where enemy peices are located at
    */

    while (knights) {
        // Popping the bits of knights so we can compute them at an order and optimally each one
        Square from = pop_lsb(knights);

        /* From emit moves, knight_attacks[from] & targets will return to us the squares
            that WE can attack, that way we can prevent adding squares where there are pieces
            of our color, its the way to prevent attacking ourselves whilst attacking the rival
        */
        emit_moves(from, knight_attacks[from] & targets, enemy, out);
    }
}

static void generate_bishop_moves(const Position& pos, std::vector<Move>& out){
    Color us = pos.side_to_move;
    Color them = static_cast<Color>(1-us);

    Piece bishop = (us == WHITE) ? W_BISHOP : B_BISHOP;

    Bitboard bishops = pos.pieces[bishop];
    Bitboard possibleTargets = ~pos.colors[us];
    Bitboard enemy = pos.colors[them];
    /* The same logic will be followed across all peices of getting those values */
    while(bishops){
        Square from = pop_lsb(bishops);

        // Same logic, it checks the empty or enemy squares and AND with the attacks the bishop
        // can have, that way ignores pieces of teh same color but sees everyhting
        Bitboard bishopAttacks = bishop_attacks(from, pos.occupied) & possibleTargets;

        emit_moves(from, bishopAttacks , enemy, out);
    }

}

static void generate_rook_moves(const Position& pos, std::vector<Move>& out){
    Color us = pos.side_to_move;
    Color them = static_cast<Color>(1-us);

    Piece rook = (us == WHITE)? W_ROOK : B_ROOK;

    Bitboard rooks = pos.pieces[rook];
    Bitboard possibleTargets = ~pos.colors[us];
    Bitboard enemy = pos.colors[them];

    while(rooks){
        Square from = pop_lsb(rooks);
        Bitboard rookAttacks = rook_attacks(from, pos.occupied) & possibleTargets;

        emit_moves(from,rookAttacks,enemy,out);
    }   
}

static void generate_king_moves(const Position& pos, std::vector<Move>& out){
    Color us   = pos.side_to_move;
    Color them = static_cast<Color>(1 - us);

    Piece king = (us == WHITE) ? W_KING : B_KING;

    Bitboard kings   = pos.pieces[king];
    Bitboard possibleTargets = ~pos.colors[us];
    Bitboard enemy   = pos.colors[them];

    while (kings) {
        Square from = pop_lsb(kings);
        Bitboard attacks = king_attacks[from] & possibleTargets;
        emit_moves(from, attacks, enemy, out);
    }
}

static void generate_queen_moves(const Position& pos, std::vector<Move>& out){
    Color us   = pos.side_to_move;
    Color them = static_cast<Color>(1 - us);

    Piece queen = (us == WHITE) ? W_QUEEN : B_QUEEN;

    Bitboard queens  = pos.pieces[queen];
    Bitboard targets = ~pos.colors[us];
    Bitboard enemy   = pos.colors[them];

    while (queens) {
        Square from = pop_lsb(queens);
        Bitboard attacks = queen_attacks(from, pos.occupied) & targets;
        emit_moves(from, attacks, enemy, out);
    }
}

/* Pawn moves are different and special since they are dependant on their position or the oens of
    other pawns
*/

static void generate_pawn_moves(const Position& pos, std::vector<Move>& out){
    Color us   = pos.side_to_move;
    Color them = static_cast<Color>(1 - us);

    Piece pawn = (us == WHITE) ? W_PAWN : B_PAWN;

    Bitboard pawns    = pos.pieces[pawn];
    Bitboard enemy    = pos.colors[them];
    Bitboard empty    = ~(pos.occupied);

    // Rank masks for promotions and initial double pushes

    /*
    0x  FF  00  00  00  00  00  00  00  <-- Rank 8 (Bits 56-63) [Promotions]
    0x  00  FF  00  00  00  00  00  00  <-- Rank 7 (Bits 48-55) [White Start / Black Promo]
    0x  00  00  FF  00  00  00  00  00  <-- Rank 6 (Bits 40-47) [Black Single Push Target]
    0x  00  00  00  FF  00  00  00  00  <-- Rank 5 (Bits 32-39)
    0x  00  00  00  00  FF  00  00  00  <-- Rank 4 (Bits 24-31)
    0x  00  00  00  00  00  FF  00  00  <-- Rank 3 (Bits 16-23) [White Single Push Target]
    0x  00  00  00  00  00  00  FF  00  <-- Rank 2 (Bits 8-15)  [White Start / Black Promo]
    0x  00  00  00  00  00  00  00  FF  <-- Rank 1 (Bits 0-7)   [Promotions]
    */

    /* The format for the hexxing numbers is
        0x -> represents it will be a hexadecimal format
        each digit represents 4 bits, since there are 16 its 64 bits
        FF -> F by defailt means 1111, and FF means we will fill two hexes of Bits and make them 1
        that way it represents the rank being filled,   also ULL to clatify it being for the uint64
    */
    Bitboard rank_7   = (us == WHITE) ? 0x00FF000000000000ULL : 0x000000000000FF00ULL;
    Bitboard rank_3   = (us == WHITE) ? 0x0000000000FF0000ULL : 0x0000FF0000000000ULL;

    int forward = (us == WHITE) ? 8 : -8;
    // Square index offset for moving forward 1 rank, also 8 cause its a bit addition

    while(pawns){
        Square from = pop_lsb(pawns);
        // Get the future pushed square
        Square push_sq = static_cast<Square>(from + forward);

        // -------------------------------------------------------------------
        // 1. PAWN PUSHES (Single & Double)
        // -------------------------------------------------------------------
        if (test_bit(empty, push_sq)) {
            // Is it a promotion push?
            if (test_bit(rank_7, from)) {
                out.push_back(make_move(from, push_sq, FLAG_PROMO_Q));
                out.push_back(make_move(from, push_sq, FLAG_PROMO_R));
                out.push_back(make_move(from, push_sq, FLAG_PROMO_B));
                out.push_back(make_move(from, push_sq, FLAG_PROMO_N));
            } else {
                out.push_back(make_move(from, push_sq, FLAG_QUIET));

                // Double Push check: must start on rank 2/7 AND target rank 3/6 empty
                Square double_push_sq = static_cast<Square>(from + 2 * forward);
                if (test_bit(rank_3, push_sq) && test_bit(empty, double_push_sq)) {
                    out.push_back(make_move(from, double_push_sq, FLAG_DOUBLE));
                }
            }
        }


        Bitboard captures = pawn_attacks[us][from] & enemy;

        while (captures) {
            Square to = pop_lsb(captures);
            if (test_bit(rank_7, from)) {
                out.push_back(make_move(from, to, FLAG_PROMO_Q));
                out.push_back(make_move(from, to, FLAG_PROMO_R));
                out.push_back(make_move(from, to, FLAG_PROMO_B));
                out.push_back(make_move(from, to, FLAG_PROMO_N));
            } else {
                out.push_back(make_move(from, to, FLAG_CAPTURE));
            }
        }

        if (pos.ep_square != -1) {
            Bitboard ep_attacks = pawn_attacks[us][from] & (1ULL << pos.ep_square);
            if (ep_attacks) {
                out.push_back(make_move(from, pos.ep_square, FLAG_EP));
            }
        }

    }
}

static void generate_castling_moves(const Position& pos, std::vector<Move>& out)
{
    Color us   = pos.side_to_move;
    Color them = static_cast<Color>(1 - us);
    // Pretty self explanatory, checks if they are busy and if any piece is attacking specific castling squares
    if (us == WHITE) {
        // White Kingside (e1 -> g1)
        if (pos.castling_rights & 8) {
            if (!test_bit(pos.occupied, SQ_F1) && !test_bit(pos.occupied, SQ_G1)) {
                if (!is_square_attacked(SQ_E1, them, pos) &&
                    !is_square_attacked(SQ_F1, them, pos) &&
                    !is_square_attacked(SQ_G1, them, pos)) 
                {
                    out.push_back(make_move(SQ_E1, SQ_G1, FLAG_CASTLE));
                }
            }
        }
        // White Queenside (e1 -> c1)
        if (pos.castling_rights & 4) {
            if (!test_bit(pos.occupied, SQ_B1) && !test_bit(pos.occupied, SQ_C1) && !test_bit(pos.occupied, SQ_D1)) {
                if (!is_square_attacked(SQ_E1, them, pos) &&
                    !is_square_attacked(SQ_D1, them, pos) &&
                    !is_square_attacked(SQ_C1, them, pos)) 
                {
                    out.push_back(make_move(SQ_E1, SQ_C1, FLAG_CASTLE));
                }
            }
        }
    } else {
        // Black Kingside (e8 -> g8)
        if (pos.castling_rights & 2) {
            if (!test_bit(pos.occupied, SQ_F8) && !test_bit(pos.occupied, SQ_G8)) {
                if (!is_square_attacked(SQ_E8, WHITE, pos) &&
                    !is_square_attacked(SQ_F8, WHITE, pos) &&
                    !is_square_attacked(SQ_G8, WHITE, pos)) 
                {
                    out.push_back(make_move(SQ_E8, SQ_G8, FLAG_CASTLE));
                }
            }
        }
        // Black Queenside (e8 -> c8)
        if (pos.castling_rights & 1) {
            if (!test_bit(pos.occupied, SQ_B8) && !test_bit(pos.occupied, SQ_C8) && !test_bit(pos.occupied, SQ_D8)) {
                if (!is_square_attacked(SQ_E8, WHITE, pos) &&
                    !is_square_attacked(SQ_D8, WHITE, pos) &&
                    !is_square_attacked(SQ_C8, WHITE, pos)) 
                {
                    out.push_back(make_move(SQ_E8, SQ_C8, FLAG_CASTLE));
                }
            }
        }
    }
}

void generate_moves(const Position& pos, std::vector<Move>& moves){
    moves.clear();
    generate_knight_moves(pos, moves);
    generate_bishop_moves(pos, moves);
    generate_rook_moves(pos, moves);
    generate_queen_moves(pos, moves);
    generate_king_moves(pos, moves);
    generate_pawn_moves(pos,moves);
    generate_castling_moves(pos,moves);

}