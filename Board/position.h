#pragma once

#include <string>
#include <vector>
#include "../Utils/types.h"


struct UndoInfo {
    Piece  captured        = PIECE_NONE;   // PIECE_NONE if no capture
    Square captured_sq     = -1;           // usually == to_sq; different for en passant
    int    castling_rights = 0;
    int    ep_square       = -1;
    int    halfmove        = 0;
};

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
    Side     side_to_move = WHITE;
    uint8_t  castling_rights = 0;   // 4 bits: WK, WQ, BK, BQ
    Square   ep_square = -1;        // -1 = none
    int      halfmove = 0;
    int      fullmove = 1;

    std::vector<UndoInfo> undo_stack; 
};

Piece pieceGetter(char c);
void put_piece(Position& pos, Piece piece, Square sq, Side color);


void parse_fen(Position& pos, const std::string& fen);
void print_board(const Position& pos);

bool is_square_attacked(Square sq, Side attacker, const Position& pos);
bool in_check(const Position& pos, Side c);