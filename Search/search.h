#pragma once
#include <chrono>
#include "../Board/position.h"
#include "../Board/move.h"

// Define boundaries larger than any possible evaluation score
constexpr int INF = 50000;
constexpr int MATE_SCORE = 49000;

// The main entry point called by your UCI loop
Move search_best_move(Position& pos, int depth);

// Searches progressively deeper until the time limit is reached.
Move search_best_move_timed(Position& pos, std::chrono::milliseconds time_limit);