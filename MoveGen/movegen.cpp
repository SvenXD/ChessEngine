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

void generate_moves(const Position& pos, std::vector<Move>& moves)
{
    moves.clear();
    generate_knight_moves(pos, moves);
}