#pragma once        //Replacemenet for indef defintions for simplifications
#include <cstdint>   // C library

using Bitboard = std::uint64_t;
using Square   = int;

enum Color{
    WHITE, BLACK
};

enum PieceType{
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,  PIECE_TYPE_NUMBER = 6, NONE = 6
};

enum Piece{
    W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    PIECE_NUMBER = 12, PIECE_NONE = 12
};

enum CastlingRights : uint8_t {
    NO_CASTLING = 0,
    WK_CASTLE = 1, // 0001
    WQ_CASTLE = 1 << 1, // 0010
    BK_CASTLE = 1 << 2, // 0100
    BQ_CASTLE = 1 << 3  // 1000
};

enum Squares : int {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    SQ_NONE = 64
};

/* Research registrees and inline theory as to why use it TODO:*/
//  TODO: Do it

//Files = columns -> a-h
// Ranks = Rows -> 1-8


inline Square make_square(int file, int rank){return (rank*8) + file;}
inline int file_of(Square square){return square & 7;} //since 7 is 111 it will check the comparizon with the number of the square
inline int rank_of(Square square){return square >> 3;} //move for example 11000 to the right will be the right rank