// tests/test_move.cpp
#include <cassert>
#include <iostream>
#include <string>

#include "../Utils/types.h"
#include "../Utils/bit_utils.h"
#include "../Board/move.h"

// ---------- tiny test harness ----------
static int tests_run    = 0;
static int tests_passed = 0;

#define CHECK(expr)                                          \
    do {                                                     \
        ++tests_run;                                         \
        if (expr) { ++tests_passed; }                        \
        else {                                               \
            std::cout << "FAIL: " #expr                      \
                      << "   (line " << __LINE__ << ")\n";   \
        }                                                    \
    } while (0)

// ---------- move packing ----------
void test_encode_decode() {
    // e2 = file 4, rank 1 -> 1*8 + 4 = 12
    // e4 = file 4, rank 3 -> 3*8 + 4 = 28
    Move m = make_move(12, 28, FLAG_QUIET);
    CHECK(from_sq(m) == 12);
    CHECK(to_sq(m)   == 28);
    CHECK(flag_of(m) == FLAG_QUIET);

    // a1 -> h8
    Move m2 = make_move(0, 63, FLAG_CAPTURE);
    CHECK(from_sq(m2) == 0);
    CHECK(to_sq(m2)   == 63);
    CHECK(flag_of(m2) == FLAG_CAPTURE);

    // Every (from,to) pair with every flag must survive packing
    for (int f = 0; f < 64; ++f) {
        for (int t = 0; t < 64; ++t) {
            Move mv = make_move(f, t, FLAG_PROMO_Q);
            CHECK(from_sq(mv) == f);
            CHECK(to_sq(mv)   == t);
            CHECK(flag_of(mv) == FLAG_PROMO_Q);
        }
    }
}

// ---------- flag predicates ----------
void test_predicates() {
    CHECK( is_capture(make_move(0, 1, FLAG_CAPTURE)));
    CHECK( is_capture(make_move(0, 1, FLAG_EP)));
    CHECK( is_capture(make_move(0, 1, FLAG_PROMO_Q)));
    CHECK( is_capture(make_move(0, 1, FLAG_PROMO_N)));
    CHECK(!is_capture(make_move(0, 1, FLAG_QUIET)));
    CHECK(!is_capture(make_move(0, 1, FLAG_DOUBLE)));
    CHECK(!is_capture(make_move(0, 1, FLAG_CASTLE)));

    CHECK( is_promotion(make_move(0, 1, FLAG_PROMO_Q)));
    CHECK( is_promotion(make_move(0, 1, FLAG_PROMO_R)));
    CHECK(!is_promotion(make_move(0, 1, FLAG_CAPTURE)));

    CHECK( is_quiet(make_move(0, 1, FLAG_QUIET)));
    CHECK( is_quiet(make_move(0, 1, FLAG_DOUBLE)));
    CHECK( is_quiet(make_move(0, 1, FLAG_CASTLE)));
    CHECK(!is_quiet(make_move(0, 1, FLAG_CAPTURE)));
    CHECK(!is_quiet(make_move(0, 1, FLAG_EP)));
    CHECK(!is_quiet(make_move(0, 1, FLAG_PROMO_Q)));
}

// ---------- string round-trip (only if you wrote move_to_string) ----------


// ---------- types.h sanity ----------
void test_square_helpers() {
    CHECK(make_square(0, 0) == 0);    // a1
    CHECK(make_square(7, 0) == 7);    // h1
    CHECK(make_square(0, 1) == 8);    // a2
    CHECK(make_square(0, 7) == 56);   // a8
    CHECK(make_square(7, 7) == 63);   // h8

    for (int s = 0; s < 64; ++s) {
        CHECK(make_square(file_of(s), rank_of(s)) == s);
    }
}

// ---------- bit_utils sanity ----------
void test_bit_utils() {
    Bitboard b = 0ULL;
    set_bit(b, 5);
    set_bit(b, 40);
    CHECK(popcount(b) == 2);
    CHECK(test_bit(b, 5));
    CHECK(test_bit(b, 40));
    CHECK(!test_bit(b, 0));

    CHECK(lsb(b) == 5);
    Square s = pop_lsb(b);
    CHECK(s == 5);
    CHECK(popcount(b) == 1);
    CHECK(lsb(b) == 40);

    clear_bit(b, 40);
    CHECK(b == 0ULL);
}

int main() {
    test_encode_decode();
    test_predicates();
    test_square_helpers();
    test_bit_utils();

    std::cout << "\n" << tests_passed << "/" << tests_run << " checks passed\n";
    if (tests_passed != tests_run) {
        std::cout << "SOME TESTS FAILED\n";
        return 1;
    }
    std::cout << "ALL TESTS PASSED\n";
    return 0;
}