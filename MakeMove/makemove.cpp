#include "makemove.h"
#include "../Board/position.h"



/* tldr just checks all the pieces and the POS utils to see which one is there*/
static Piece piece_on(const Position& pos, Square sq) {
    for (int p = 0; p < 12; ++p)
        if (test_bit(pos.pieces[p], sq)) return static_cast<Piece>(p);
    return PIECE_NONE;
}

void make_move(Position& pos, Move m) {
    Square   from = from_sq(m);
    Square   to   = to_sq(m);
    MoveFlag flag = flag_of(m);

    // Remember since its 16 bits, Move already stores the final positions, the starter and what
    // the move was about

    Color us   = pos.side_to_move;
    Color them = static_cast<Color>(1 - us);

    // 1. Save undo information before mutating anything.
    UndoInfo u;
    u.castling_rights = pos.castling_rights;
    u.ep_square       = pos.ep_square;
    u.halfmove        = pos.halfmove;

    // Figure out which piece is moving.
    Piece moving = piece_on(pos, from);

    // 2. Handle captures (normal + promotion-capture).
    if (flag == FLAG_CAPTURE || (flag >= FLAG_PROMO_N && flag <= FLAG_PROMO_Q)) {
        Piece captured = piece_on(pos, to);
        if (captured != PIECE_NONE) {
            u.captured    = captured;
            u.captured_sq = to;
            pos.pieces[captured] &= ~(1ULL << to);
            pos.colors[them]     &= ~(1ULL << to);
            pos.occupied         &= ~(1ULL << to);
        }
    }

    // 3. En passant: remove the captured pawn, which is *behind* `to`.
    if (flag == FLAG_EP) {
        Square cap_sq = (us == WHITE) ? (Square)(to - 8) : (Square)(to + 8);
        Piece their_pawn = (us == WHITE) ? B_PAWN : W_PAWN;
        u.captured    = their_pawn;
        u.captured_sq = cap_sq;
        pos.pieces[their_pawn] &= ~(1ULL << cap_sq);
        pos.colors[them]       &= ~(1ULL << cap_sq);
        pos.occupied           &= ~(1ULL << cap_sq);
    }

    // 4. Remove the moving piece from `from`.
    pos.pieces[moving] &= ~(1ULL << from);
    pos.colors[us]     &= ~(1ULL << from);
    pos.occupied       &= ~(1ULL << from);

    // 5. Place it on `to` (possibly as a promoted piece).
    Piece placed = moving;
    if (flag >= FLAG_PROMO_N && flag <= FLAG_PROMO_Q) {
        PieceType pt;
        switch (flag) {
            case FLAG_PROMO_N: pt = KNIGHT; break;
            case FLAG_PROMO_B: pt = BISHOP; break;
            case FLAG_PROMO_R: pt = ROOK;   break;
            default:           pt = QUEEN;  break;
        }
        placed = (us == WHITE) ? static_cast<Piece>(W_PAWN + pt)
                               : static_cast<Piece>(B_PAWN + pt);
    }
    pos.pieces[placed] |= (1ULL << to);
    pos.colors[us]     |= (1ULL << to);
    pos.occupied       |= (1ULL << to);

    // 6. Castling: also move the rook.
    if (flag == FLAG_CASTLE) {
        Square rook_from, rook_to;
        Piece  rook = (us == WHITE) ? W_ROOK : B_ROOK;
        if (to > from) {              // kingside
            rook_from = (us == WHITE) ? 7  : 63;   // h1 / h8
            rook_to   = (us == WHITE) ? 5  : 61;   // f1 / f8
        } else {                       // queenside
            rook_from = (us == WHITE) ? 0  : 56;   // a1 / a8
            rook_to   = (us == WHITE) ? 3  : 59;   // d1 / d8
        }
        pos.pieces[rook] &= ~(1ULL << rook_from);
        pos.colors[us]   &= ~(1ULL << rook_from);
        pos.occupied     &= ~(1ULL << rook_from);

        pos.pieces[rook] |= (1ULL << rook_to);
        pos.colors[us]   |= (1ULL << rook_to);
        pos.occupied     |= (1ULL << rook_to);
    }

    // 7. Update castling rights.
    //   - King moves: both of that side's rights are gone.
    //   - Rook moves from its home square: that side's corresponding right is gone.
    //   - A rook is captured on its home square: enemy's right is gone.
    if (moving == W_KING) pos.castling_rights &= ~(WK_CASTLE | WQ_CASTLE);
    if (moving == B_KING) pos.castling_rights &= ~(BK_CASTLE | BQ_CASTLE);
    if (from == 0  || to == 0 ) pos.castling_rights &= ~WQ_CASTLE; // a1
    if (from == 7  || to == 7 ) pos.castling_rights &= ~WK_CASTLE; // h1
    if (from == 56 || to == 56) pos.castling_rights &= ~BQ_CASTLE; // a8
    if (from == 63 || to == 63) pos.castling_rights &= ~BK_CASTLE; // h8

    // 8. En passant square for next ply.
    if (flag == FLAG_DOUBLE) {
        pos.ep_square = (us == WHITE) ? (Square)(from + 8) : (Square)(from - 8);
    } else {
        pos.ep_square = -1;
    }

    // 9. Halfmove clock: reset on pawn move or capture, else increment.
    bool is_pawn  = (moving == W_PAWN || moving == B_PAWN);
    bool captured = (u.captured != PIECE_NONE);
    if (is_pawn || captured) pos.halfmove = 0;
    else                     pos.halfmove += 1;

    // 10. Flip side; increment fullmove if black just moved.
    pos.side_to_move = them;
    if (us == BLACK) pos.fullmove += 1;

    // 11. Record undo entry.
    pos.undo_stack.push_back(u);
}


void unmake_move(Position& pos, Move m) {
    Square   from = from_sq(m);
    Square   to   = to_sq(m);
    MoveFlag flag = flag_of(m);

    // Restore side to the mover before we do anything else.
    Color them = pos.side_to_move;              // currently the opponent
    Color us   = static_cast<Color>(1 - them);  // the side that just moved
    pos.side_to_move = us;

    if (us == BLACK) pos.fullmove -= 1;

    UndoInfo u = pos.undo_stack.back();
    pos.undo_stack.pop_back();

    // 1. Undo the piece placement on `to`.
    //    If it was a promotion, the piece on `to` is a Q/R/B/N; we need
    //    to remove it and restore the pawn on `from`.
    if (flag >= FLAG_PROMO_N && flag <= FLAG_PROMO_Q) {
        PieceType pt;
        switch (flag) {
            case FLAG_PROMO_N: pt = KNIGHT; break;
            case FLAG_PROMO_B: pt = BISHOP; break;
            case FLAG_PROMO_R: pt = ROOK;   break;
            default:           pt = QUEEN;  break;
        }
        Piece promoted = (us == WHITE) ? static_cast<Piece>(W_PAWN + pt)
                                       : static_cast<Piece>(B_PAWN + pt);
        pos.pieces[promoted] &= ~(1ULL << to);
        pos.colors[us]       &= ~(1ULL << to);
        pos.occupied         &= ~(1ULL << to);

        Piece pawn = (us == WHITE) ? W_PAWN : B_PAWN;
        pos.pieces[pawn] |= (1ULL << from);
        pos.colors[us]   |= (1ULL << from);
        pos.occupied     |= (1ULL << from);
    } else {
        Piece moving = piece_on(pos, to);
        pos.pieces[moving] &= ~(1ULL << to);
        pos.colors[us]     &= ~(1ULL << to);
        pos.occupied       &= ~(1ULL << to);

        pos.pieces[moving] |= (1ULL << from);
        pos.colors[us]     |= (1ULL << from);
        pos.occupied       |= (1ULL << from);
    }

    // 2. Undo castling rook move.
    if (flag == FLAG_CASTLE) {
        Square rook_from, rook_to;
        Piece  rook = (us == WHITE) ? W_ROOK : B_ROOK;
        if (to > from) {
            rook_from = (us == WHITE) ? 7  : 63;
            rook_to   = (us == WHITE) ? 5  : 61;
        } else {
            rook_from = (us == WHITE) ? 0  : 56;
            rook_to   = (us == WHITE) ? 3  : 59;
        }
        pos.pieces[rook] &= ~(1ULL << rook_to);
        pos.colors[us]   &= ~(1ULL << rook_to);
        pos.occupied     &= ~(1ULL << rook_to);

        pos.pieces[rook] |= (1ULL << rook_from);
        pos.colors[us]   |= (1ULL << rook_from);
        pos.occupied     |= (1ULL << rook_from);
    }

    // 3. Restore captured piece (normal capture, promotion-capture, or ep).
    if (u.captured != PIECE_NONE) {
        Color cap_color = (u.captured >= W_PAWN && u.captured <= W_KING) ? WHITE : BLACK;
        pos.pieces[u.captured] |= (1ULL << u.captured_sq);
        pos.colors[cap_color]  |= (1ULL << u.captured_sq);
        pos.occupied           |= (1ULL << u.captured_sq);
    }

    // 4. Restore scalar state.
    pos.castling_rights = u.castling_rights;
    pos.ep_square       = u.ep_square;
    pos.halfmove        = u.halfmove;
}