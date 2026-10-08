#pragma once
#include <cstdint>
#include <string>
#include "../Utils/types.h"

/* This files utility is to create the bit values and definitions of a move */

using Move = uint16_t;

// Flag bits (top 4 bits of the 16-bit move)
// Types of moves each one of them exactly types of moves
// A Move is a 16-bit packed encoding:
//   bits  0-5 : from square (0-63)
//   bits  6-11 : to square   (0-63)
//   bits 12-15 : flags (4 bits, see MoveFlag)

// 6 bits = 64 spaces, 0-63     4 bits the type of move since its 8


enum MoveFlag : uint16_t {
    FLAG_QUIET      = 0,
    FLAG_CAPTURE    = 1,
    FLAG_DOUBLE     = 2,   // pawn double push (for ep square tracking)
    FLAG_EP         = 3,   // en passant capture
    FLAG_CASTLE     = 4,
    FLAG_PROMO_N    = 5,
    FLAG_PROMO_B    = 6,
    FLAG_PROMO_R    = 7,
    FLAG_PROMO_Q    = 8,
};

/* The way to store the move data into the 16 bits 

    from will occupy the first 6 bits with itself and or it with
    to which will be the next 6 and finally flag which will be the type of move
    into the final 4, totalling

    ....|......|......   16 bits for each one data
*/
inline Move make_move(Square from, Square to, MoveFlag flag = FLAG_QUIET) {
    return (Move)(from | (to << 6) | (flag << 12));
}

/* Works as an eraser the 0x3f (hexa) to only get back the first 6 bits, the same with the rest*/
inline Square   from_sq(Move m) { return m & 0x3F; }
inline Square   to_sq(Move m)   { return (m >> 6) & 0x3F; }
inline MoveFlag flag_of(Move m) { return (MoveFlag)(m >> 12); }

inline bool is_capture(Move m) {
    MoveFlag f = flag_of(m);
    return f == FLAG_CAPTURE || f == FLAG_EP || (f >= FLAG_PROMO_N);
}

inline bool is_promotion(Move m) { return flag_of(m) >= FLAG_PROMO_N; }

inline bool is_quiet(Move m) {
    MoveFlag f = flag_of(m);
    return f == FLAG_QUIET || f == FLAG_DOUBLE || f == FLAG_CASTLE;
}

std::string move_to_string(Move m); 