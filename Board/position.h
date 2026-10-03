#pragma once

#include <string>
#include "../Utils/types.h"

/**
 * @brief Template for the current state of a game.
 * 
 * Pieces an array of 12 being the 64 bits to state where each piece (black, white king etc) is at
 * Colors indicate where ALL the white and ALL the black pieces are at
 */
struct Position {
    Bitboard pieces[12] = {};   // one per Piece enum
    Bitboard colors[2]  = {};   // OR of all white / all black
    Bitboard occupied   = 0;    // OR of everything
    Color    side_to_move = WHITE;
    uint8_t  castling_rights = 0;   // 4 bits: WK, WQ, BK, BQ
    Square   ep_square = -1;        // -1 = none
    int      halfmove = 0;
    int      fullmove = 1;
};

Piece pieceGetter(char c);
void put_piece(Position& pos, Piece piece, Square sq, Color color);
void parse_fen(Position& pos, const std::string& fen);
void print_board(const Position& pos);