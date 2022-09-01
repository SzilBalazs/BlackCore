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

constexpr Score kingMgPSQT[64] = {
        -48, -58, -68, -68, -68, -68, -58, -48,
        -38, -58, -58, -58, -60, -58, -58, -38,
        -28, -38, -42, -38, -38, -38, -38, -28,
        -18, -23, -30, -32, -28, -28, -23, -18,
        -5, -14, -17, -18, -18, -13, -8, -5,
        1, -4, -4, -14, -14, -4, -4, 1,
        36, 36, 30, 6, 6, 26, 36, 40,
        36, 52, 32, -4, -4, 26, 52, 36,
};

constexpr Score kingEgPSQT[64] = {
        -54, -44, -34, -34, -34, -34, -44, -54,
        -44, -24, 1, 1, 1, 1, -24, -44,
        -34, 1, 6, 11, 11, 6, 1, -34,
        -34, 1, 11, 16, 16, 11, 1, -34,
        -34, 7, 11, 16, 16, 11, 1, -34,
        -28, 7, 12, 17, 17, 6, 5, -28,
        -38, -18, 7, 7, 7, 3, -18, -38,
        -48, -38, -34, -28, -28, -28, -44, -48,
};

constexpr Score pawnMgPSQT[64] = {
        12, 12, 12, 12, 12, 12, 12, 12,
        15, 15, 18, 19, 19, 18, 15, 15,
        14, 14, 15, 17, 17, 15, 14, 14,
        7, 7, 15, 19, 19, 15, 7, 5,
        7, 7, 15, 22, 22, 13, 3, 3,
        6, 1, 11, 13, 13, 15, 1, 10,
        7, 7, 15, -24, -24, 15, 7, 7,
        12, 12, 12, 12, 12, 12, 12, 12,
};

constexpr Score pawnEgPSQT[64] = {
        -4, -4, -4, -4, -4, -4, -4, -4,
        -1, -1, 2, 3, 3, 2, -1, -1,
        -2, -2, -1, 1, 1, -1, -2, -2,
        -9, -9, -1, 3, 3, -1, -9, -9,
        -9, -9, -1, 6, 6, -1, -9, -9,
        -6, -9, -1, 5, 5, -1, -9, -6,
        -9, -9, -1, -28, -28, -1, -9, -9,
        -4, -4, -4, -4, -4, -4, -4, -4,
};

constexpr Score knightMgPSQT[64] = {
        -8, -5, 2, 2, -2, 0, -3, -8,
        -5, 5, 6, 7, 7, 6, 5, -3,
        2, 4, 17, 17, 17, 17, 6, 2,
        -2, 3, 17, 22, 22, 17, 3, -2,
        -2, 5, 13, 20, 22, 13, 3, -2,
        -2, 2, 17, 13, 13, 17, 2, -2,
        -7, 1, 2, 3, 3, 2, 1, -7,
        -22, -17, -17, -17, -17, -17, -17, -22,
};

constexpr Score knightEgPSQT[64] = {
        -24, -15, -14, -10, -10, -14, -15, -24,
        -15, -11, -6, -7, -7, -10, -11, -19,
        -14, -6, 1, 1, 1, 1, -6, -14,
        -10, -5, 1, 6, 6, 1, -5, -10,
        -10, -5, 5, 10, 10, 5, -5, -10,
        -10, -6, 3, 5, 5, 5, -6, -10,
        -15, -7, -6, -5, -5, -6, -7, -15,
        -30, -25, -25, -25, -25, -25, -25, -30,
};

constexpr Score bishopMgPSQT[64] = {
        2, 0, 2, 2, 2, 2, 2, 2,
        -3, 12, 12, 12, 12, 12, 12, 2,
        2, 12, 15, 15, 15, 15, 12, 2,
        2, 12, 15, 14, 14, 15, 12, 2,
        2, 12, 15, 14, 14, 15, 12, -2,
        -2, 13, 15, 14, 14, 15, 13, -2,
        -7, 15, 13, 13, 13, 13, 15, 2,
        -22, -17, -17, -17, -17, -17, -17, -22,
};

constexpr Score bishopEgPSQT[64] = {
        -14, -12, -12, -14, -14, -14, -14, -12,
        -15, -4, -4, -4, -4, -4, -4, -12,
        -14, -4, -1, -1, -1, -1, -4, -14,
        -14, -4, -1, -2, -2, -1, -4, -14,
        -14, -4, -1, -2, -2, -1, -4, -14,
        -10, -3, -1, -2, -2, -1, -3, -10,
        -19, 3, -3, -3, -3, -3, 7, -14,
        -30, -25, -25, -25, -25, -25, -25, -30,
};

constexpr Score rookMgPSQT[64] = {
        13, 16, 16, 16, 14, 12, 14, 11,
        14, 17, 17, 17, 17, 17, 17, 14,
        7, 12, 12, 12, 12, 12, 12, 7,
        7, 12, 12, 12, 12, 12, 12, 7,
        7, 12, 12, 12, 12, 8, 12, 7,
        3, 8, 8, 10, 10, 8, 8, 3,
        3, 10, 8, 10, 8, 8, 8, 3,
        8, 12, 12, 17, 17, 12, 8, 8,
};

constexpr Score rookEgPSQT[64] = {
        3, 4, 2, 2, 2, 2, 2, 1,
        -2, 1, 1, 1, 1, 1, 1, -2,
        -9, -4, -4, -4, -4, -4, -4, -9,
        -9, -4, -4, -4, -4, -4, -4, -9,
        -5, -4, -4, -4, -4, -4, -4, -9,
        -5, 0, 0, -4, -2, -2, 0, -5,
        -5, 0, 0, -2, 0, 0, 0, -5,
        0, 0, 0, 1, 1, -2, 0, 0,
};

constexpr Score queenMgPSQT[64] = {
        9, 9, 9, 9, 9, 9, 9, 9,
        12, 12, 12, 12, 9, 9, 9, 9,
        12, 12, 12, 12, 12, 12, 12, 12,
        12, 12, 13, 13, 13, 13, 12, 12,
        12, 12, 13, 15, 15, 13, 12, 12,
        12, 12, 13, 15, 15, 13, 12, 12,
        12, 12, 14, 15, 15, 14, 12, 12,
        7, 7, 7, 7, 7, 7, 7, 7,
};

constexpr Score queenEgPSQT[64] = {
        -4, -4, -4, -4, -4, -4, -4, -4,
        -4, -4, -4, -4, -4, -4, -4, -4,
        -4, -4, -4, -4, -4, -4, -4, -4,
        -4, -4, -3, -3, -3, -3, -4, -4,
        -4, -4, -3, -1, -1, -3, -4, -4,
        -4, -4, -3, -1, -1, -3, -4, -4,
        -4, -4, -2, -1, -1, -2, -4, -4,
        -9, -9, -9, -9, -9, -9, -9, -9,
};


#endif //BLACKCORE_EVAL_H
