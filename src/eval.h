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
        -67, -72, -81, -82, -79, -82, -67, -59,
        -50, -74, -72, -74, -69, -66, -76, -51,
        -38, -53, -52, -54, -55, -52, -55, -42,
        -24, -39, -42, -48, -48, -48, -43, -25,
        -12, -23, -24, -29, -36, -28, -22, -11,
        2, -3, -14, -20, -15, -8, -4, 2,
        37, 33, 16, 5, 5, 22, 37, 31,
        37, 32, 14, -2, -2, 28, 32, 38,
};

constexpr Score kingEgPSQT[64] = {
        -58, -48, -36, -36, -33, -38, -33, -58,
        -46, -28, -3, -3, -3, -1, -21, -48,
        -32, -3, 2, 7, 7, 2, -3, -33,
        -34, 1, 7, 12, 12, 7, -3, -35,
        -27, -1, 14, 15, 12, 7, -3, -24,
        -22, 6, 14, 22, 20, 6, 2, -24,
        -32, -12, 5, 13, 13, 5, -17, -32,
        -42, -37, -24, -22, -22, -22, -48, -42,
};

constexpr Score pawnMgPSQT[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        -1, -5, -2, -1, -1, -2, -5, -5,
        -6, -6, -5, -3, -3, -5, -6, -6,
        -13, -13, -5, -1, -1, -5, -13, -12,
        -13, -13, -5, 2, 2, -5, -13, -7,
        -4, -1, -1, 13, 13, -5, -1, -10,
        -7, -10, -5, -22, -22, -5, -11, -13,
        0, 0, 0, 0, 0, 0, 0, 0,
};

constexpr Score pawnEgPSQT[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        -5, -5, -2, -1, -1, -2, -5, -5,
        -6, -6, -5, -3, -3, -5, -6, -6,
        -13, -13, -5, -1, -1, -5, -13, -13,
        -13, -13, -5, 2, 2, -5, -13, -13,
        -10, -13, -5, 1, 1, -5, -13, -10,
        -13, -13, -5, -22, -22, -5, -13, -13,
        0, 0, 0, 0, 0, 0, 0, 0,
};

constexpr Score knightMgPSQT[64] = {
        -28, -23, -18, -18, -18, -18, -21, -28,
        -17, -15, -12, -13, -13, -14, -15, -23,
        -18, -14, -3, -3, -3, -3, -14, -18,
        -4, -7, -3, 2, 2, -3, 0, -14,
        -3, -13, 7, 2, 2, 11, -11, -3,
        -2, 2, 4, 13, 11, 4, 2, -2,
        -7, 1, 2, 3, 3, 2, 1, -7,
        -22, -17, -17, -17, -17, -17, -17, -22,
};

constexpr Score knightEgPSQT[64] = {
        -28, -11, -18, -9, -16, -18, -15, -28,
        -11, -13, -3, -13, -6, -14, -14, -16,
        -15, -14, -3, -3, -3, -3, -8, -14,
        -11, -3, -3, 2, 2, -3, -9, -7,
        -2, -11, 4, 3, 2, 2, -10, -2,
        -2, 2, 8, 5, 6, 13, 2, -2,
        -7, 0, 1, 3, 3, 2, 1, -7,
        -24, -17, -17, -17, -17, -17, -17, -22,
};

constexpr Score bishopMgPSQT[64] = {
        -17, -17, -17, -17, -17, -17, -10, -15,
        -19, -7, -7, -7, -7, -7, -7, -17,
        -17, -7, -4, -4, -4, -4, -7, -17,
        -15, -7, -4, -5, -5, -4, -7, -17,
        -12, -7, -4, -5, -5, -4, -7, -8,
        -5, -6, -4, -5, -5, -4, -6, -11,
        -15, 9, -7, -7, -7, -5, 12, -17,
        -22, -17, -17, -17, -17, -17, -17, -22,
};

constexpr Score bishopEgPSQT[64] = {
        -5, -15, -17, -17, -9, -14, -3, -10,
        -13, -7, -7, -7, -7, -7, -7, -17,
        -16, -7, -4, -4, -4, -4, -7, -15,
        -17, -7, -4, -5, -5, -4, -7, -16,
        -7, -7, -4, -5, -5, -4, -7, -16,
        -9, -5, -4, -5, -5, -4, -2, -7,
        -10, 2, -3, -6, -6, -1, 3, -15,
        -23, -18, -18, -20, -19, -18, -18, -23,
};

constexpr Score rookMgPSQT[64] = {
        -4, -3, -3, -3, -3, -3, -3, -3,
        -5, -2, -2, -2, -2, -2, -2, -5,
        -12, -7, -7, -7, -7, -7, -7, -12,
        -12, -7, -7, -7, -7, -7, -7, -7,
        -12, -4, -7, -7, -7, -7, -7, 0,
        2, 0, -7, -7, 0, -2, 3, 2,
        0, 0, 7, 0, 0, 1, 6, 1,
        0, -7, 0, 8, 8, 0, -4, 0,
};

constexpr Score rookEgPSQT[64] = {
        7, 4, 5, 6, 6, 5, 7, 9,
        -5, -1, -2, -2, -2, -2, -2, -5,
        -12, -7, -7, -7, -7, -7, -7, -12,
        -12, -7, -7, -7, -7, -7, -7, -12,
        -8, -7, -7, -7, -7, -7, -7, -12,
        2, -5, -6, -7, -4, -5, 7, 2,
        2, 6, 4, -3, 1, -4, -1, -1,
        7, 4, 6, 4, 4, 1, 6, 7,
};

constexpr Score queenMgPSQT[64] = {
        -6, -6, -6, -6, -6, -6, -6, -6,
        -4, -6, -6, -6, -6, -6, -6, -6,
        -6, -6, -6, -6, -6, -6, -5, -6,
        -4, -6, -5, -5, -5, -5, -6, -6,
        4, -6, -5, -3, -3, -5, -6, -4,
        -5, -6, -5, -3, 1, -5, -6, -3,
        3, 0, 2, 2, 1, 2, 0, -4,
        1, 1, 1, 1, 1, 1, 1, -2,
};

constexpr Score queenEgPSQT[64] = {
        -5, -5, -5, -5, -5, -5, -5, -5,
        1, -4, -5, -5, -5, -5, -5, -3,
        -5, -5, -5, -5, -5, -5, -5, -5,
        -3, -3, -4, -4, -4, -4, -4, -5,
        -1, -5, -3, -2, -2, -3, -4, 2,
        -4, -3, -3, -1, 2, -1, -5, -5,
        5, 0, 3, 4, 7, 3, -2, -1,
        -1, -2, 0, 1, 1, -1, -1, -1,
};

#endif //BLACKCORE_EVAL_H
