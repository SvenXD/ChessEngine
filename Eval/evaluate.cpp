#include "evaluate.h"
#include "pst.h"
#include <algorithm> // for std::min

// Material values in centipawns
constexpr int PAWN_VALUE   = 100;
constexpr int KNIGHT_VALUE = 300;
constexpr int BISHOP_VALUE = 300;
constexpr int ROOK_VALUE   = 500;
constexpr int QUEEN_VALUE  = 900;

// Phase weights (max 24 at the start of a game)
constexpr int KNIGHT_PHASE = 1;
constexpr int BISHOP_PHASE = 1;
constexpr int ROOK_PHASE   = 2;
constexpr int QUEEN_PHASE  = 4;

// Helper to pop the least significant bit (gets square index 0-63)
inline Square pop_lsb(uint64_t& bb) {
    Square sq = __builtin_ctzll(bb);
    bb &= bb - 1; // Clear the bit
    return sq;
}

// Lookup helpers
inline int get_pst_white(const int pst[64], Square sq) { return pst[sq ^ 56]; }
inline int get_pst_black(const int pst[64], Square sq) { return pst[sq]; }

int evaluate(const Position& pos) {
    int mg_score = 0;
    int eg_score = 0;
    int game_phase = 0;

    // --- PAWNS ---
    uint64_t wp = pos.pieces[W_PAWN];
    while (wp) {
        Square sq = pop_lsb(wp);
        mg_score += PAWN_VALUE + get_pst_white(PST::MG_Pawn, sq);
        eg_score += PAWN_VALUE + get_pst_white(PST::EG_Pawn, sq);
    }
    uint64_t bp = pos.pieces[B_PAWN];
    while (bp) {
        Square sq = pop_lsb(bp);
        mg_score -= PAWN_VALUE + get_pst_black(PST::MG_Pawn, sq);
        eg_score -= PAWN_VALUE + get_pst_black(PST::EG_Pawn, sq);
    }

    // --- KNIGHTS ---
    uint64_t wn = pos.pieces[W_KNIGHT];
    while (wn) {
        Square sq = pop_lsb(wn);
        mg_score += KNIGHT_VALUE + get_pst_white(PST::MG_Knight, sq);
        eg_score += KNIGHT_VALUE + get_pst_white(PST::EG_Knight, sq);
        game_phase += KNIGHT_PHASE;
    }
    uint64_t bn = pos.pieces[B_KNIGHT];
    while (bn) {
        Square sq = pop_lsb(bn);
        mg_score -= KNIGHT_VALUE + get_pst_black(PST::MG_Knight, sq);
        eg_score -= KNIGHT_VALUE + get_pst_black(PST::EG_Knight, sq);
        game_phase += KNIGHT_PHASE;
    }

    // --- BISHOPS ---
    uint64_t wb = pos.pieces[W_BISHOP];
    while (wb) {
        Square sq = pop_lsb(wb);
        mg_score += BISHOP_VALUE + get_pst_white(PST::MG_Bishop, sq);
        eg_score += BISHOP_VALUE + get_pst_white(PST::EG_Bishop, sq);
        game_phase += BISHOP_PHASE;
    }
    uint64_t bb = pos.pieces[B_BISHOP];
    while (bb) {
        Square sq = pop_lsb(bb);
        mg_score -= BISHOP_VALUE + get_pst_black(PST::MG_Bishop, sq);
        eg_score -= BISHOP_VALUE + get_pst_black(PST::EG_Bishop, sq);
        game_phase += BISHOP_PHASE;
    }

    // --- ROOKS ---
    uint64_t wr = pos.pieces[W_ROOK];
    while (wr) {
        Square sq = pop_lsb(wr);
        mg_score += ROOK_VALUE + get_pst_white(PST::MG_Rook, sq);
        eg_score += ROOK_VALUE + get_pst_white(PST::EG_Rook, sq);
        game_phase += ROOK_PHASE;
    }
    uint64_t br = pos.pieces[B_ROOK];
    while (br) {
        Square sq = pop_lsb(br);
        mg_score -= ROOK_VALUE + get_pst_black(PST::MG_Rook, sq);
        eg_score -= ROOK_VALUE + get_pst_black(PST::EG_Rook, sq);
        game_phase += ROOK_PHASE;
    }

    // --- QUEENS ---
    uint64_t wq = pos.pieces[W_QUEEN];
    while (wq) {
        Square sq = pop_lsb(wq);
        mg_score += QUEEN_VALUE + get_pst_white(PST::MG_Queen, sq);
        eg_score += QUEEN_VALUE + get_pst_white(PST::EG_Queen, sq);
        game_phase += QUEEN_PHASE;
    }
    uint64_t bq = pos.pieces[B_QUEEN];
    while (bq) {
        Square sq = pop_lsb(bq);
        mg_score -= QUEEN_VALUE + get_pst_black(PST::MG_Queen, sq);
        eg_score -= QUEEN_VALUE + get_pst_black(PST::EG_Queen, sq);
        game_phase += QUEEN_PHASE;
    }

    // --- KINGS --- (Kings have no material value, only PST)
    Square wk_sq = __builtin_ctzll(pos.pieces[W_KING]);
    mg_score += get_pst_white(PST::MG_King, wk_sq);
    eg_score += get_pst_white(PST::EG_King, wk_sq);

    Square bk_sq = __builtin_ctzll(pos.pieces[B_KING]);
    mg_score -= get_pst_black(PST::MG_King, bk_sq);
    eg_score -= get_pst_black(PST::EG_King, bk_sq);

    // --- TAPERED BLEND ---
    game_phase = std::min(game_phase, 24); // Cap at 24 in case of early promotions
    
    int mg_weight = game_phase;
    int eg_weight = 24 - game_phase;
    
    int score = ((mg_score * mg_weight) + (eg_score * eg_weight)) / 24;

    // Return relative to the side to move
    return (pos.side_to_move == WHITE) ? score : -score;
}   