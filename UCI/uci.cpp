// uci.cpp
#include "uci.h"
#include "../Board/position.h"
#include "../Board/move.h"
#include "../MoveGen/movegen.h"
#include "../MakeMove/makemove.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Forward declaration of parse_FEN if it's not in position.h
extern void parse_FEN(Position& pos, const std::string& fen);
extern uint64_t perft_divide(Position& pos, int depth);

// Helper: Converts a string like "e2e4" or "e7e8q" into your engine's Move object
static Move parse_move(const std::string& move_str, const Position& pos) {
    std::vector<Move> moves;
    generate_moves(pos, moves);

    int source_file = move_str[0] - 'a';
    int source_rank = move_str[1] - '1';
    int target_file = move_str[2] - 'a';
    int target_rank = move_str[3] - '1';

    Square source_sq = static_cast<Square>(source_rank * 8 + source_file);
    Square target_sq = static_cast<Square>(target_rank * 8 + target_file);

    for (const auto& move : moves) {
        if (from_sq(move) == source_sq && to_sq(move) == target_sq) {
            // Check for promotion character if present
            if (move_str.length() == 5) {
                int flag = move & 0xF000; // Assuming standard 16-bit move layout; adjust if needed
                char promo = move_str[4];
                if ((promo == 'q' && flag == FLAG_PROMO_Q) ||
                    (promo == 'r' && flag == FLAG_PROMO_R) ||
                    (promo == 'b' && flag == FLAG_PROMO_B) ||
                    (promo == 'n' && flag == FLAG_PROMO_N)) {
                    return move;
                }
                continue;
            }
            return move;
        }
    }
    return 0; // Invalid or illegal move
}

void parse_position(const std::string& command, Position& pos) {
    std::istringstream ss(command);
    std::string token;
    ss >> token; // Skip "position"

    ss >> token;
    if (token == "startpos") {
        parse_FEN(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        ss >> token; // Consume "moves" if it exists
    } else if (token == "fen") {
        std::string fen = "";
        while (ss >> token && token != "moves") {
            fen += token + " ";
        }
        parse_FEN(pos, fen);
    }

    // Apply any moves provided after the position setup
    while (ss >> token) {
        Move m = parse_move(token, pos);
        if (m != 0) {
            make_move(pos, m);
        }
    }
}

void parse_go(const std::string& command, Position& pos) {
    std::istringstream ss(command);
    std::string token;
    int depth = -1;

    // Basic parsing for "go depth X" or "go perft X"
    while (ss >> token) {
        if (token == "depth" || token == "perft") {
            ss >> depth;
        }
    }

    if (depth == -1) {
        depth = 5; // Default depth if none provided
    }

    // For now, since search isn't built, route 'go' to perft so you can test it in GUIs
    perft_divide(pos, depth);
    
    // When you build Search, replace perft_divide with:
    // Move best_move = search(pos, depth);
    // std::cout << "bestmove " << move_to_string(best_move) << std::endl;
}

void uci_loop() {
    Position pos;
    parse_FEN(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    std::string line;
    std::cout << "id name MyChessEngine\n";
    std::cout << "id author You\n";
    std::cout << "uciok\n";

    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::string command;
        ss >> command;

        if (command == "uci") {
            std::cout << "id name MyChessEngine\n";
            std::cout << "id author You\n";
            std::cout << "uciok\n";
        } 
        else if (command == "isready") {
            std::cout << "readyok\n";
        } 
        else if (command == "ucinewgame") {
            parse_FEN(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        } 
        else if (command == "position") {
            parse_position(line, pos);
        } 
        else if (command == "go") {
            parse_go(line, pos);
        } 
        else if (command == "quit") {
            break;
        }
    }
}