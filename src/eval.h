//     BlackCore is a UCI Chess engine
//     Copyright (c) 2022 SzilBalazs
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef BLACKCORE_EVAL_H
#define BLACKCORE_EVAL_H

#include "constants.h"
#include "position.h"

Score eval(const Position &pos);

struct Value {
    Score mg = 0;
    Score eg = 0;

    inline Value operator+(Value a) const { return {mg + a.mg, eg + a.eg}; }

    inline Value operator-(Value a) const { return {mg - a.mg, eg - a.eg}; }

    inline Value operator*(int a) const { return {mg * a, eg * a}; }

    inline Value &operator+=(Value a) {
        mg += a.mg;
        eg += a.eg;
        return *this;
    }

    inline Value &operator+=(Score a) {
        mg += a;
        eg += a;
        return *this;
    }
};

struct EvalData {

};

#ifdef TUNE

extern Value PIECE_VALUES[6];

extern Score TEMPO_SCORE;

extern Value PAWN_PASSED_BONUS;
extern Value PAWN_DOUBLE_PENALTY;
extern Value PAWN_ISOLATED_PENALTY;

extern Value KNIGHT_MOBILITY;

extern Value BISHOP_ATTACK_BONUS;

extern Value ROOK_MOBILITY;
extern Value ROOK_TRAPPED;
extern Value ROOK_OPEN_BONUS;
extern Value ROOK_HALF_BONUS;

extern Value KING_UNSAFE;
extern Value KING_SHIELD_1;
extern Value KING_SHIELD_2;

#else

constexpr Value PIECE_VALUES[6] = {{0,    0},
                                   {98,   139},
                                   {394,  378},
                                   {450,  530},
                                   {647,  821},
                                   {1300, 1700}};


constexpr Score TEMPO_SCORE = 10;
constexpr Score SPACE_SCORE = 3;

constexpr Value PAWN_PASSED_BONUS = {29, 58};
constexpr Value PAWN_DOUBLE_PENALTY = {-4, -21};
constexpr Value PAWN_ISOLATED_PENALTY = {-13, -28};

constexpr Value KNIGHT_MOBILITY = {11, 10};

constexpr Value BISHOP_MOBILITY = {10, 10};

constexpr Value ROOK_MOBILITY = {7, 2};
constexpr Value ROOK_TRAPPED = {-70, -25};
constexpr Value ROOK_OPEN_BONUS = {34, 19};
constexpr Value ROOK_HALF_BONUS = {17, 21};

constexpr Value KING_SHIELD_1 = {25, 0};
constexpr Value KING_SHIELD_2 = {15, 0};

#endif

void initEval();

constexpr Bitboard WK_AREA = 0xe0e0e0ULL;
constexpr Bitboard WK_SHIELD_1 = 0xe000ULL;
constexpr Bitboard WK_SHIELD_2 = 0xe00000ULL;

constexpr Bitboard WQ_AREA = 0x70707ULL;
constexpr Bitboard WQ_SHIELD_1 = 0x700ULL;
constexpr Bitboard WQ_SHIELD_2 = 0x70000ULL;

constexpr Bitboard BK_AREA = 0xe0e0e00000000000ULL;
constexpr Bitboard BK_SHIELD_1 = 0xe0000000000000ULL;
constexpr Bitboard BK_SHIELD_2 = 0xe00000000000ULL;

constexpr Bitboard BQ_AREA = 0x707070000000000ULL;
constexpr Bitboard BQ_SHIELD_1 = 0x7000000000000ULL;
constexpr Bitboard BQ_SHIELD_2 = 0x70000000000ULL;

constexpr Score pawnPSQT[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        3, 3, 6, 7, 7, 6, 3, 3,
        2, 2, 3, 5, 5, 3, 2, 2,
        -5, -5, 3, 7, 7, 3, -5, -5,
        -5, -5, 3, 10, 10, 3, -5, -5,
        -2, -5, 3, 5, 5, 3, -5, -2,
        -5, -5, 3, -30, -30, 3, -5, -5,
        0, 0, 0, 0, 0, 0, 0, 0
};

constexpr Score knightPSQT[64] = {
        -20, -15, -10, -10, -10, -10, -15, -20,
        -15, -7, -6, -5, -5, -6, -7, -15,
        -10, -6, 5, 5, 5, 5, -6, -10,
        -10, -5, 5, 10, 10, 5, -5, -10,
        -10, -5, 5, 10, 10, 5, -5, -10,
        -10, -6, 5, 5, 5, 5, -6, -10,
        -15, -7, -6, -5, -5, -6, -7, -15,
        -30, -25, -25, -25, -25, -25, -25, -30,
};


constexpr Score bishopPSQT[64] = {
        -10, -10, -10, -10, -10, -10, -10, -10,
        -15, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 3, 3, 3, 3, 0, -10,
        -10, 0, 3, 2, 2, 3, 0, -10,
        -10, 0, 3, 2, 2, 3, 0, -10,
        -10, 1, 3, 2, 2, 3, 1, -10,
        -15, 7, 1, 1, 1, 1, 7, -10,
        -30, -25, -25, -25, -25, -25, -25, -30,
};

constexpr Score rookPSQT[64] = {
        3, 4, 4, 4, 4, 4, 4, 3,
        2, 5, 5, 5, 5, 5, 5, 2,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        0, 0, 0, 5, 5, 0, 0, 0,
};

constexpr Score queenPSQT[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 1, 1, 0, 0,
        0, 0, 1, 3, 3, 1, 0, 0,
        0, 0, 1, 3, 3, 1, 0, 0,
        0, 0, 2, 3, 3, 2, 0, 0,
        -5, -5, -5, -5, -5, -5, -5, -5
};

constexpr Score kingPSQT_mg[64] = {
        -60, -70, -80, -80, -80, -80, -70, -60,
        -50, -70, -70, -70, -70, -70, -70, -50,
        -40, -50, -50, -50, -50, -50, -50, -40,
        -30, -35, -40, -40, -40, -40, -35, -30,
        -15, -20, -25, -30, -30, -25, -20, -15,
        -5, -10, -10, -20, -20, -10, -10, -5,
        30, 30, 20, 0, 0, 20, 30, 30,
        30, 40, 20, -10, -10, 20, 40, 30
};

constexpr Score kingPSQT_eg[64] = {-50, -40, -30, -30, -30, -30, -40, -50,
                                   -40, -20, 5, 5, 5, 5, -20, -40,
                                   -30, 5, 10, 15, 15, 10, 5, -30,
                                   -30, 5, 15, 20, 20, 15, 5, -30,
                                   -30, 5, 15, 20, 20, 15, 5, -30,
                                   -30, 5, 10, 15, 15, 10, 5, -30,
                                   -40, -20, 5, 5, 5, 5, -20, -40,
                                   -50, -40, -30, -30, -30, -30, -40, -50};

#endif //BLACKCORE_EVAL_H
