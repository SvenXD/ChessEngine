// uci.cpp
#include "uci.h"
#include "../Board/position.h"
#include "../Board/move.h"
#include "../MoveGen/movegen.h"
#include "../MakeMove/makemove.h"
#include "../Search/search.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Forward declaration of parse_fen if it's not in position.h
extern void parse_fen(Position& pos, const std::string& fen);
//extern uint64_t perft_divide(Position& pos, int depth);

// Helper: Converts a string like "e2e4" or "e7e8q" into your engine's Move object
Move parse_move(const std::string& move_str, const Position& pos) {
    if (move_str.length() != 4 && move_str.length() != 5) {
        return 0;
    }

    if (move_str[0] < 'a' || move_str[0] > 'h' ||
        move_str[1] < '1' || move_str[1] > '8' ||
        move_str[2] < 'a' || move_str[2] > 'h' ||
        move_str[3] < '1' || move_str[3] > '8') {
        return 0;
    }

    MoveFlag requested_promotion = FLAG_QUIET;
    if (move_str.length() == 5) {
        switch (move_str[4]) {
            case 'q': requested_promotion = FLAG_PROMO_Q; break;
            case 'r': requested_promotion = FLAG_PROMO_R; break;
            case 'b': requested_promotion = FLAG_PROMO_B; break;
            case 'n': requested_promotion = FLAG_PROMO_N; break;
            default: return 0;
        }
    }

    std::vector<Move> moves;
    generate_moves(pos, moves);

    int source_file = move_str[0] - 'a';
    int source_rank = move_str[1] - '1';
    int target_file = move_str[2] - 'a';
    int target_rank = move_str[3] - '1';

    Square source_sq = static_cast<Square>(source_rank * 8 + source_file);
    Square target_sq = static_cast<Square>(target_rank * 8 + target_file);

    for (const auto& move : moves) {
        if (from_sq(move) == source_sq && to_sq(move) == target_sq &&
            ((move_str.length() == 5 && flag_of(move) == requested_promotion) ||
             (move_str.length() == 4 && !is_promotion(move)))) {
            return move;
        }
    }
    return 0; // Invalid or illegal move
}

// In UCI/uci.cpp
void parse_position(const std::string& command, Position& pos) {
    std::istringstream ss(command);
    std::string token;
    ss >> token; // Skip "position"

    ss >> token;
    if (token == "startpos") {
        parse_fen(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        ss >> token; // Consume "moves" if present
    } else if (token == "fen") {
        std::string fen = "";
        while (ss >> token && token != "moves") {
            fen += token + " ";
        }
        parse_fen(pos, fen);
    }

    // Apply moves provided after the position setup
    while (ss >> token) {
        Move m = parse_move(token, pos);
        if (m == 0) {
            std::cerr << "[ERROR] Failed to parse move string: '" << token << "'\n";
            break; // Stop parsing to prevent board state corruption
        }
        make_move(pos, m);
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

    Move best = search_best_move_timed(pos, std::chrono::milliseconds(1000));
    // or: Move best = search(pos, depth);

    if (best == 0) {
        // No legal move (checkmate / stalemate) — some GUIs expect a null reply.
        std::cout << "bestmove 0000" << std::endl;
    } else {
        std::cout << "bestmove " << move_to_string(best) << std::endl;
    }
}

void uci_loop() {
    Position pos;
    parse_fen(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

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
            parse_fen(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
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