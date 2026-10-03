#include "move.h"
/* This files utility is to translate into an actual notation to pass*/


/* Fast lookup between 0-7 of index*/
static const char* file_chars = "abcdefgh";

std::string move_to_string(Move m) {
    std::string s;
    /* Using utils for the files and ranks to have the notation */
    s += file_chars[file_of(from_sq(m))];
    s += char('1' + rank_of(from_sq(m)));
    s += file_chars[file_of(to_sq(m))];
    s += char('1' + rank_of(to_sq(m)));

    /* Switch for the final 4 bits of the move to check what type of move*/
    switch (flag_of(m)) {
        case FLAG_PROMO_N: s += 'n'; break;
        case FLAG_PROMO_B: s += 'b'; break;
        case FLAG_PROMO_R: s += 'r'; break;
        case FLAG_PROMO_Q: s += 'q'; break;
        default: break;
    }
    return s;   // e.g. "e2e4", "e7e8q"
}