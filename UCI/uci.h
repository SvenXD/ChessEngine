#pragma once 
#include "../Board/move.h"
#include "../Board/position.h"
#include <string>

// Starts the main UCI loop to listen for GUI commands
void uci_loop();

// Parses the "position" command to set up the board and play moves
void parse_position(const std::string& command, Position& pos);

// Parses the "go" command to start thinking (or run perft)
void parse_go(const std::string& command, Position& pos);

Move parse_move(const std::string& move_str, const Position& pos);