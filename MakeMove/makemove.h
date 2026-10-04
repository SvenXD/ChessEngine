#pragma once
#include "../Board/position.h"
#include "../Board/move.h"
#include "../Utils/bit_utils.h"

void make_move  (Position& pos, Move m);
void unmake_move(Position& pos, Move m);