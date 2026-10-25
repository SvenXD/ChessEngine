#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <string>
#include <sstream>

// Include headers matching your project structure
#include "Board/position.h"
#include "Board/move.h"
#include "MoveGen/movegen.h"
#include "MakeMove/makemove.h"
#include "Attacks/precomputed.h"
#include "UCI/uci.h"

inline void put_piece(Position& pos, Piece piece, Square sq, Side color) {
    pos.pieces[piece] |= (1ULL << sq);
    pos.colors[color] |= (1ULL << sq);
    pos.occupied      |= (1ULL << sq);
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

void parse_FEN(Position& pos, const std::string& fen){

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
            Side color = (piece >= W_PAWN && piece <= W_KING) ? WHITE : BLACK;
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

// ---------------------------------------------------------------------------
// Core Recursive Perft Function
// ---------------------------------------------------------------------------
uint64_t perft(Position& pos, int depth) {
    if (depth == 0) return 1ULL;

    uint64_t nodes = 0;
    std::vector<Move> move_list;
    
    // Generate moves for current position
    generate_moves(pos, move_list);

    for (const auto& move : move_list) {
        make_move(pos, move);

        Side side_that_moved = static_cast<Side>(1 - pos.side_to_move);
        
        // Only count the nodes if the move was strictly legal
        if (!in_check(pos, side_that_moved)) {
            nodes += perft(pos, depth - 1); 
        }

        unmake_move(pos, move);
    }

    return nodes;
}

// ---------------------------------------------------------------------------
// Perft Root with Divide (Prints leaf nodes per root move)
// ---------------------------------------------------------------------------
uint64_t perft_divide(Position& pos, int depth) {
    if (depth == 0) return 1ULL;

    uint64_t total_nodes = 0;
    std::vector<Move> move_list;
    generate_moves(pos, move_list);

    std::cout << "\n---------------- PERFT DIVIDE (Depth " << depth << ") ----------------\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    for (const auto& move : move_list) {
        make_move(pos, move);

        Side side_that_moved = static_cast<Side>(1 - pos.side_to_move);
        if (!in_check(pos, side_that_moved)) {
            uint64_t nodes = perft(pos, depth - 1);
            total_nodes += nodes;
            
            std::cout << "Move: " << from_sq(move) << " -> " << to_sq(move)
                      << " | Nodes: " << nodes << "\n";
        }

        unmake_move(pos, move);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "---------------------------------------------------\n";
    std::cout << "Total Nodes : " << total_nodes << "\n";
    std::cout << "Time Elapsed: " << std::fixed << std::setprecision(3) << elapsed.count() << " s\n";
    if (elapsed.count() > 0) {
        uint64_t nps = static_cast<uint64_t>(total_nodes / elapsed.count());
        std::cout << "NPS         : " << nps << " nodes/sec\n";
    }
    std::cout << "---------------------------------------------------\n\n";

    return total_nodes;
}

// ---------------------------------------------------------------------------
// Engine Entry Point
// ---------------------------------------------------------------------------
int main() {
    // 1. Initialize attack tables and magic bitboards
    init_attack_tables();

    // 2. Hand control over to the UCI protocol loop
    uci_loop();

    return 0;
}