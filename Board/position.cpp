#include "position.h"
#include "../Attacks/precomputed.h"
#include "../Attacks/sliding.h"
#include <sstream>

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

void parse_fen(Position& pos, const std::string& fen){

    pos = Position();

    std::stringstream ss(fen);   
    std::string boardStr, sideStr, castlingStr, epStr;

    ss >> boardStr >> sideStr >> castlingStr >> epStr;
    int rank = 7, file = 0;

    for(char c: boardStr){
        if(c == '/'){
            rank--;
            file = 0;
        }else if(isdigit(c)){
            file += c-'0';
        }else{
            Piece piece = pieceGetter(c);
            Color color = (piece >= W_PAWN && piece <= W_KING) ? WHITE : BLACK;
            Square sq   = (rank * 8) + file;
            put_piece(pos, piece, sq, color);
            file++;
        }
    }

    pos.side_to_move = (sideStr == "w") ? WHITE : BLACK;

    if (castlingStr != "-") {
        for (char c : castlingStr) {
            switch (c) {
                case 'K': pos.castling_rights |= WK_CASTLE; break;
                case 'Q': pos.castling_rights |= WQ_CASTLE; break;
                case 'k': pos.castling_rights |= BK_CASTLE; break;
                case 'q': pos.castling_rights |= BQ_CASTLE; break;
            }
        }
    }

    if(epStr != "-"){
        pos.ep_square = ((epStr[1] - '1') * 8) + (epStr[0] - 'a');
    }else{
        pos.ep_square = -1;
    }
}

Piece pieceGetter(char c) {
    switch (c) {
        // Black Pieces
        case 'p': return B_PAWN;
        case 'n': return B_KNIGHT;
        case 'b': return B_BISHOP;
        case 'r': return B_ROOK;
        case 'q': return B_QUEEN;
        case 'k': return B_KING;

        // White Pieces
        case 'P': return W_PAWN;
        case 'N': return W_KNIGHT;
        case 'B': return W_BISHOP;
        case 'R': return W_ROOK;
        case 'Q': return W_QUEEN;
        case 'K': return W_KING;

        default:  return PIECE_NONE; // Sentinel value for invalid input or empty squares
    }
}

void put_piece(Position& pos, Piece piece, Square sq, Color color) {
    pos.pieces[piece] |= (1ULL << sq);
    pos.colors[color] |= (1ULL << sq);
    pos.occupied      |= (1ULL << sq);
}