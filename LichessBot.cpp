#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>  // For popen() and fgets()
#include <memory>
#include <chrono>
#include "Attacks/precomputed.h"
#include "Board/position.h"
#include "Board/move.h"
#include "MakeMove/makemove.h"
#include "Search/search.h"
#include "UCI/uci.h"
#include <sstream>

const std::string API_TOKEN = "lip_B4AMI5o9xA7l954BDifg"; 

void accept_challenge(const std::string& challenge_id) {
    std::string command = "curl -s -X POST https://lichess.org/api/challenge/" + challenge_id + "/accept "
                          "-H \"Authorization: Bearer " + API_TOKEN + "\"";
    
    std::cout << "Sending accept request...\n";
    system(command.c_str());
    std::cout << "\nDone.\n";
}

// Your existing play_move function
void play_move(const std::string& game_id, const std::string& move) {
    std::string command = "curl -s -X POST https://lichess.org/api/bot/game/" + game_id + "/move/" + move + " "
                          "-H \"Authorization: Bearer " + API_TOKEN + "\"";
    system(command.c_str());
}

// Añade el parámetro del color con el que juega tu bot (WHITE o BLACK)
void auto_play_game(const std::string& game_id, Side my_color) {
    std::string command = "curl -sN https://lichess.org/api/bot/game/stream/" + game_id + " "
                          "-H \"Authorization: Bearer " + API_TOKEN + "\"";

#ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif

    if (!pipe) return;

    Position pos;
    char buffer[2048];

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        
        size_t moves_pos = line.find("\"moves\":\"");
        if (moves_pos != std::string::npos) {
            size_t start = moves_pos + 9;
            size_t end = line.find("\"", start);
            std::string moves_str = line.substr(start, end - start);

            // Rebuild board from startpos + full moves history
            std::string uci_cmd = "position startpos moves " + moves_str;
            parse_position(uci_cmd, pos);

            // Only make a move if it is strictly our turn
            if (pos.side_to_move == my_color) {
                std::cout << "Engine thinking...\n";
                Move best_move = search_best_move_timed(pos, std::chrono::seconds(5));
                if (best_move == 0) {
                    std::cerr << "No legal move found; not sending a move.\n";
                    continue;
                }
                std::string move_text = move_to_string(best_move);

                std::cout << "Calculated move: " << move_text << "\n";
                play_move(game_id, move_text);
            }
        }
    }

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
}

int main() {
    init_attack_tables();

    std::cout << "=== C++ Lichess Manual Bridge ===\n";
    std::cout << "Commands:\n";
    std::cout << "  accept <challenge_id>\n";
    std::cout << "  move <game_id> <move>\n";
    std::cout << "  quit\n\n";

    std::string action;
    while (true) {
        std::cout << "> ";
        std::cin >> action;

        if (action == "quit") {
            break;
        } 
        else if(action == "accept"){
            std::string challenge_id;
            std::cin >> challenge_id;
            accept_challenge(challenge_id);
        }
        else if (action == "stream") {
            std::string game_id;
            std::cin >> game_id;
            auto_play_game(game_id,WHITE);
        } 
        else {
            std::cout << "Unknown command.\n";
        }
    }

    return 0;
}