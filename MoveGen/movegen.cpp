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
        Square to = pop_lsb(attacks);
        MoveFlag flag = test_bit(enemy, to) ? FLAG_CAPTURE : FLAG_QUIET;
        out.push_back(make_move(from, to, flag));
    }
}

static void generate_knight_moves(const Position& pos, std::vector<Move>& out)
{
    Color us    = pos.side_to_move;
    Color them  = static_cast<Color>(1 - us);

    Piece our_knight = (us == WHITE) ? W_KNIGHT : B_KNIGHT;

    Bitboard knights = pos.pieces[our_knight];
    Bitboard targets = ~pos.colors[us];   // empty or enemy squares
    Bitboard enemy   = pos.colors[them];

    while (knights) {
        Square from = pop_lsb(knights);
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

    while(bishops){
        Square from = pop_lsb(bishops);
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
    Bitboard rank_7   = (us == WHITE) ? 0x00FF000000000000ULL : 0x000000000000FF00ULL;
    Bitboard rank_3   = (us == WHITE) ? 0x00000000FF000000ULL : 0x000000FF00000000ULL;

    int forward = (us == WHITE) ? 8 : -8; // Square index offset for moving forward 1 rank, also 8 cause its a bit addition

    while(pawns){
        Square from = pop_lsb(pawns);
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
                out.push_back(make_move(from, to, FLAG_PROMO_B));
                out.push_back(make_move(from, to, FLAG_PROMO_N));
                out.push_back(make_move(from, to, FLAG_PROMO_R));
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

void generate_moves(const Position& pos, std::vector<Move>& moves){
    moves.clear();
    generate_knight_moves(pos, moves);
    generate_knight_moves(pos, moves);
    generate_bishop_moves(pos, moves);
    generate_rook_moves(pos, moves);
    generate_queen_moves(pos, moves);
    generate_king_moves(pos, moves);
    generate_pawn_moves(pos,moves);

}