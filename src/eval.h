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

#ifdef TUNE

extern Value PIECE_VALUES[6];

extern Score TEMPO_SCORE;

extern Value PAWN_PASSED_BONUS;
extern Value PAWN_DOUBLE_PENALTY;
extern Value PAWN_ISOLATED_PENALTY;

extern Value KNIGHT_MOBILITY;

extern Value BISHOP_MOBILITY;

extern Value ROOK_MOBILITY;
extern Value ROOK_TRAPPED;
extern Value ROOK_OPEN_BONUS;
extern Value ROOK_HALF_BONUS;

extern Value KING_SHIELD_1;
extern Value KING_SHIELD_2;

extern Value PSQT[2][6][64];

#else

constexpr Value PIECE_VALUES[6] = {{0,    0},
                                   {97,   135},
                                   {395,  379},
                                   {449,  529},
                                   {646,  820},
                                   {1283, 1576}};


constexpr Score TEMPO_SCORE = 10;

constexpr Value PAWN_PASSED_BONUS = {27, 57};
constexpr Value PAWN_DOUBLE_PENALTY = {-5, -22};
constexpr Value PAWN_ISOLATED_PENALTY = {-14, -29};

constexpr Value KNIGHT_MOBILITY = {10, 11};

constexpr Value BISHOP_MOBILITY = {14, 4};

constexpr Value ROOK_MOBILITY = {6, 1};
constexpr Value ROOK_TRAPPED = {-69, -24};
constexpr Value ROOK_OPEN_BONUS = {33, 18};
constexpr Value ROOK_HALF_BONUS = {16, 20};

constexpr Value KING_SHIELD_1 = {24, -1};
constexpr Value KING_SHIELD_2 = {14, -1};

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

constexpr Score kingMgPSQT[64] = {
        -61, -70, -78, -81, -78, -82, -71, -61,
        -51, -72, -71, -71, -71, -68, -72, -50,
        -40, -49, -51, -51, -50, -50, -51, -42,
        -29, -37, -41, -42, -42, -42, -37, -29,
        -14, -21, -26, -30, -32, -24, -22, -16,
        -3, -8, -8, -21, -18, -9, -9, -3,
        31, 30, 22, 2, 2, 20, 32, 32,
        32, 38, 18, -8, -8, 22, 38, 32,
};

constexpr Score kingEgPSQT[64] = {
        -52, -42, -30, -32, -32, -32, -39, -52,
        -42, -22, 3, 3, 3, 3, -22, -42,
        -32, 3, 8, 13, 13, 8, 3, -32,
        -32, 7, 13, 18, 18, 13, 3, -31,
        -28, 3, 15, 18, 18, 13, 3, -30,
        -28, 7, 12, 17, 14, 9, 5, -28,
        -38, -18, 7, 7, 7, 3, -19, -38,
        -48, -39, -30, -28, -28, -28, -42, -48,
};

constexpr Score pawnMgPSQT[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 4, 5, 5, 4, 1, 1,
        0, 0, 1, 3, 3, 1, 0, 0,
        -7, -7, 1, 5, 5, 1, -7, -7,
        -7, -7, 1, 8, 8, 1, -7, -4,
        0, -3, 5, 7, 7, 1, -3, -4,
        -7, -7, 1, -28, -28, 1, -7, -7,
        0, 0, 0, 0, 0, 0, 0, 0,
};

constexpr Score pawnEgPSQT[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 4, 5, 5, 4, 1, 1,
        0, 0, 1, 3, 3, 1, 0, 0,
        -7, -7, 1, 5, 5, 1, -7, -7,
        -7, -7, 1, 8, 8, 1, -7, -7,
        -4, -7, 1, 7, 7, 1, -7, -4,
        -7, -7, 1, -28, -28, 1, -7, -7,
        0, 0, 0, 0, 0, 0, 0, 0,
};

constexpr Score knightMgPSQT[64] = {
        -22, -17, -12, -12, -12, -12, -17, -22,
        -15, -9, -8, -7, -7, -8, -9, -17,
        -12, -8, 3, 3, 3, 3, -8, -12,
        -8, -7, 3, 8, 8, 3, -3, -12,
        -8, -7, 7, 8, 8, 7, -7, -8,
        -8, -4, 3, 7, 7, 3, -4, -8,
        -13, -5, -4, -3, -3, -4, -5, -13,
        -28, -23, -23, -23, -23, -23, -23, -28,
};

constexpr Score knightEgPSQT[64] = {
        -22, -17, -12, -10, -10, -12, -15, -22,
        -15, -9, -8, -7, -7, -8, -8, -16,
        -9, -8, 3, 3, 3, 3, -5, -9,
        -8, -6, 3, 8, 8, 3, -4, -10,
        -8, -7, 3, 8, 8, 3, -7, -8,
        -8, -4, 3, 6, 7, 7, -4, -8,
        -13, -5, -4, -3, -3, -4, -5, -13,
        -28, -23, -23, -23, -23, -23, -23, -28,
};

constexpr Score bishopMgPSQT[64] = {
        -11, -11, -11, -11, -11, -11, -11, -11,
        -15, -1, -1, -1, -1, -1, -1, -11,
        -11, -1, 2, 2, 2, 2, -1, -11,
        -9, -1, 2, 1, 1, 2, -1, -11,
        -11, -1, 2, 1, 1, 2, -1, -11,
        -9, 0, 2, 1, 1, 2, 0, -9,
        -13, 9, -1, -1, -1, -1, 9, -12,
        -28, -23, -23, -23, -23, -23, -23, -28,
};

constexpr Score bishopEgPSQT[64] = {
        -11, -9, -11, -11, -11, -11, -9, -9,
        -14, -1, -1, -1, -1, -1, -1, -11,
        -11, -1, 2, 2, 2, 2, -1, -11,
        -11, -1, 2, 1, 1, 2, -1, -11,
        -11, -1, 2, 1, 1, 2, -1, -10,
        -9, 0, 2, 1, 1, 2, 0, -9,
        -15, 8, 0, 0, 0, 0, 6, -11,
        -29, -24, -24, -24, -24, -24, -24, -29,
};

constexpr Score rookMgPSQT[64] = {
        2, 3, 3, 3, 3, 3, 3, 2,
        1, 4, 4, 4, 4, 4, 4, 1,
        -6, -1, -1, -1, -1, -1, -1, -6,
        -6, -1, -1, -1, -1, -1, -1, -6,
        -6, -1, -1, -1, -1, -1, -1, -4,
        -4, 1, -1, -1, 1, 0, 0, -4,
        -4, -1, 1, -1, 0, 1, 1, -4,
        1, -1, -1, 4, 4, -1, 1, 1,
};

constexpr Score rookEgPSQT[64] = {
        2, 3, 3, 3, 3, 3, 3, 3,
        1, 4, 4, 4, 4, 4, 4, 1,
        -6, -1, -1, -1, -1, -1, -1, -6,
        -6, -1, -1, -1, -1, -1, -1, -6,
        -6, -1, -1, -1, -1, -1, -1, -6,
        -4, -1, -1, -1, 0, 1, 1, -4,
        -4, 1, 1, -1, -1, 1, 1, -4,
        1, 0, 1, 4, 4, 0, 1, 1,
};

constexpr Score queenMgPSQT[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 1, 1, 0, 0,
        0, 0, 1, 3, 3, 1, 0, 0,
        0, 0, 1, 3, 3, 1, 0, 0,
        0, 0, 2, 3, 3, 2, 0, 0,
        -5, -5, -5, -5, -5, -5, -5, -5,
};

constexpr Score queenEgPSQT[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 1, 1, 0, 0,
        0, 0, 1, 3, 3, 1, 0, 0,
        0, 0, 1, 3, 3, 1, 0, 0,
        0, 0, 2, 3, 3, 2, 0, 0,
        -5, -5, -5, -5, -5, -5, -5, -5,
};

#endif //BLACKCORE_EVAL_H
