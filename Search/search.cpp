#include "search.h"
#include "../Eval/evaluate.h"
#include "../MoveGen/movegen.h"
#include "../MakeMove/makemove.h"
#include <chrono>
#include <iostream>
#include <vector>

namespace {

struct SearchControl {
    std::chrono::steady_clock::time_point deadline;
    std::uint64_t nodes = 0;
    bool stopped = false;

    bool time_expired() {
        ++nodes;
        if ((nodes == 1 || (nodes & 255) == 0) &&
            std::chrono::steady_clock::now() >= deadline) {
            stopped = true;
        }
        return stopped;
    }
};

bool negamax(Position& pos, int depth, int alpha, int beta,
             SearchControl& control, int& result) {
    if (control.time_expired()) {
        return false;
    }

    if (depth == 0) {
        result = evaluate(pos);
        return true;
    }

    std::vector<Move> moves;
    generate_moves(pos, moves);

    int legal_moves_count = 0;
    int best_score = -INF;

    for (const auto& move : moves) {
        make_move(pos, move);

        Side side_that_moved = static_cast<Side>(1 - pos.side_to_move);
        if (in_check(pos, side_that_moved)) {
            unmake_move(pos, move);
            continue;
        }

        ++legal_moves_count;
        int child_score;
        bool completed = negamax(pos, depth - 1, -beta, -alpha, control, child_score);
        unmake_move(pos, move);
        if (!completed) {
            return false;
        }

        int score = -child_score;
        if (score > best_score) {
            best_score = score;
        }
        if (score > alpha) {
            alpha = score;
        }
        if (alpha >= beta) {
            break;
        }
    }

    if (legal_moves_count == 0) {
        result = in_check(pos, pos.side_to_move) ? -MATE_SCORE + depth : 0;
        return true;
    }

    result = best_score;
    return true;
}

Move search_best_move_impl(Position& pos, int max_depth,
                           std::chrono::steady_clock::time_point deadline) {
    std::vector<Move> moves;
    generate_moves(pos, moves);

    Move best_move = 0;
    for (const auto& move : moves) {
        make_move(pos, move);
        Side side_that_moved = static_cast<Side>(1 - pos.side_to_move);
        bool legal = !in_check(pos, side_that_moved);
        unmake_move(pos, move);
        if (legal) {
            best_move = move;
            break;
        }
    }

    if (best_move == 0) {
        return 0;
    }

    SearchControl control{deadline};
    int best_score = -INF;

    for (int depth = 1; depth <= max_depth; ++depth) {
        Move iteration_best_move = 0;
        int iteration_best_score = -INF;
        int alpha = -INF;
        bool completed_iteration = true;

        for (const auto& move : moves) {
            make_move(pos, move);
            Side side_that_moved = static_cast<Side>(1 - pos.side_to_move);
            if (in_check(pos, side_that_moved)) {
                unmake_move(pos, move);
                continue;
            }

            int child_score;
            bool completed = negamax(pos, depth - 1, -INF, -alpha, control, child_score);
            unmake_move(pos, move);
            if (!completed) {
                completed_iteration = false;
                break;
            }

            int score = -child_score;
            if (score > iteration_best_score) {
                iteration_best_score = score;
                iteration_best_move = move;
            }
            if (score > alpha) {
                alpha = score;
            }
        }

        if (!completed_iteration) {
            break;
        }

        best_move = iteration_best_move;
        best_score = iteration_best_score;
        std::cout << "info depth " << depth << " score cp " << best_score << std::endl;
    }

    return best_move;
}

} // namespace

Move search_best_move(Position& pos, int depth) {
    return search_best_move_impl(pos, depth, std::chrono::steady_clock::time_point::max());
}

Move search_best_move_timed(Position& pos, std::chrono::milliseconds time_limit) {
    return search_best_move_impl(pos, 2147483647,
                                 std::chrono::steady_clock::now() + time_limit);
}
