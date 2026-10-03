#pragma once
#include <vector>
#include "../Board/position.h"
#include "../Board/move.h"

// Fills `moves` with all pseudo-legal moves for the side to move.
// Clears `moves` first. Caller should reserve ~256 entries.
void generate_moves(const Position& pos, std::vector<Move>& moves);