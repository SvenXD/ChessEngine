#include "position.h"
#include "../Attacks/precomputed.h"
#include "../Attacks/sliding.h"

bool is_square_attacked(Square sq, Color attacker, const Position& pos) {
    Color defender = static_cast<Color>(1 - attacker);

    // Pawn: a pawn on square X attacks sq if sq is in pawn_attacks[attacker][X].
    // Equivalently, look from the defender's perspective and check
    // the attacker's pawn bitboard.
    Piece atk_pawn = (attacker == WHITE) ? W_PAWN : B_PAWN;
    if (pawn_attacks[defender][sq] & pos.pieces[atk_pawn]) return true;

    // Knight
    Piece atk_knight = (attacker == WHITE) ? W_KNIGHT : B_KNIGHT;
    if (knight_attacks[sq] & pos.pieces[atk_knight]) return true;

    // King
    Piece atk_king = (attacker == WHITE) ? W_KING : B_KING;
    if (king_attacks[sq] & pos.pieces[atk_king]) return true;

    // Bishop / Queen on diagonals
    Piece atk_bishop = (attacker == WHITE) ? W_BISHOP : B_BISHOP;
    Piece atk_queen  = (attacker == WHITE) ? W_QUEEN  : B_QUEEN;
    Bitboard diag = pos.pieces[atk_bishop] | pos.pieces[atk_queen];
    if (bishop_attacks(sq, pos.occupied) & diag) return true;

    // Rook / Queen on orthogonals
    Piece atk_rook = (attacker == WHITE) ? W_ROOK : B_ROOK;
    Bitboard orth = pos.pieces[atk_rook] | pos.pieces[atk_queen];
    if (rook_attacks(sq, pos.occupied) & orth) return true;

    return false;
}

bool in_check(const Position& pos, Color c) {
    Piece king = (c == WHITE) ? W_KING : B_KING;
    Square ksq = lsb(pos.pieces[king]);
    return is_square_attacked( ksq, static_cast<Color>(1 - c), pos);
}