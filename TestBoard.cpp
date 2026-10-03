#include <iostream>
#include <cassert>
#include <sstream>

#include "Utils/bit_utils.h"
#include "Utils/types.h"

#include "Board/position.h"

using namespace std;

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

// Add a piece to a square
inline void put_piece(Position& pos, Piece piece, Square sq, Color color) {
    pos.pieces[piece] |= (1ULL << sq);
    pos.colors[color] |= (1ULL << sq);
    pos.occupied      |= (1ULL << sq);
}

// Remove a piece from a square
inline void remove_piece(Position& pos, Piece piece, Square sq, Color color) {
    pos.pieces[piece] &= ~(1ULL << sq);
    pos.colors[color] &= ~(1ULL << sq);
    pos.occupied      &= ~(1ULL << sq);
}

// Move a piece using XOR bitwise trick
inline void move_piece(Position& pos, Piece piece, Square from, Square to, Color color) {
    uint64_t move_mask = (1ULL << from) | (1ULL << to);
    pos.pieces[piece] ^= move_mask;
    pos.colors[color] ^= move_mask;
    pos.occupied      ^= move_mask;
}

void parse_FEN(Position& pos, const string& fen){

    pos = Position();

    stringstream ss(fen);   
    string boardStr, sideStr, castlingStr, epStr;

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

int main(){

    //Files = columns -> a-h
    // Ranks = Rows -> 1-8

    Position test_pos;

    string FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

    parse_FEN(test_pos,FEN);

    for(int i = 0; i < 64; i++){

        int rank = 7 - (i / 8);
        int file = i % 8;
        int square = rank * 8 + file;

        string v = " ";
        for(int j = 0; j < 12; j++){
            if(test_bit(test_pos.pieces[j],square)){
                v = to_string(j);
                break;
            }
        }

        cout << "[" << v << "]";

        if((i + 1) % 8 == 0){
            cout << endl;
        }
    }


    /*jknbjh jhfgcxdfghjvgdfg
    Bitboard b = 0ULL;
    set_bit(b,5); set_bit(b,40);

    assert(popcount(b) == 2);
    assert(lsb(b) == 5);

    Square s = pop_lsb(b); assert(s == 5); assert(popcount(b) == 1);
*/
    return 0;
}