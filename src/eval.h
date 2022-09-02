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
                                   {92,   133},
                                   {398,  382},
                                   {444,  522},
                                   {641,  815},
                                   {1294, 1694}};


constexpr Score TEMPO_SCORE = 10;

constexpr Value PAWN_PASSED_BONUS = {23, 52};
constexpr Value PAWN_DOUBLE_PENALTY = {-10, -27};
constexpr Value PAWN_ISOLATED_PENALTY = {-7, -32};

constexpr Value KNIGHT_MOBILITY = {5, 13};

constexpr Value BISHOP_MOBILITY = {4, 4};

constexpr Value ROOK_MOBILITY = {1, -4};
constexpr Value ROOK_TRAPPED = {-64, -19};
constexpr Value ROOK_OPEN_BONUS = {28, 13};
constexpr Value ROOK_HALF_BONUS = {11, 17};

constexpr Value KING_SHIELD_1 = {19, -6};
constexpr Value KING_SHIELD_2 = {13, -6};

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
        -49, -62, -67, -67, -66, -72, -61, -41,
        -40, -58, -58, -57, -60, -56, -53, -35,
        -22, -41, -43, -42, -41, -40, -42, -32,
        -18, -27, -31, -27, -31, -31, -23, -14,
        -2, -9, -12, -18, -15, -8, -6, 0,
        13, 8, 4, -6, -6, 7, 7, 13,
        48, 47, 30, 16, 17, 37, 46, 47,
        48, 48, 29, 8, 8, 38, 48, 48,
};

constexpr Score kingEgPSQT[64] = {
        -66, -56, -46, -42, -46, -46, -56, -66,
        -52, -36, -11, -11, -11, -11, -34, -52,
        -44, -11, -6, -1, -1, -6, -11, -46,
        -46, -11, -1, 8, 4, -1, -11, -42,
        -36, -1, 3, 18, 4, -1, -9, -35,
        -16, 17, 16, 25, 25, 9, 9, -16,
        -26, -6, 11, 19, 18, 3, -9, -26,
        -36, -26, -37, -16, -16, -16, -53, -36,
};

constexpr Score pawnMgPSQT[64] = {
        18, 18, 18, 18, 18, 18, 18, 18,
        12, 12, 15, 16, 17, 16, 13, 13,
        11, 11, 12, 14, 14, 12, 11, 11,
        4, 4, 12, 16, 16, 12, 4, 6,
        4, 4, 12, 19, 19, 15, 0, 12,
        13, 8, 18, 23, 24, 12, 12, 7,
        3, 3, 11, -12, -13, 12, 4, 4,
        18, 18, 18, 18, 18, 18, 18, 18,
};

constexpr Score pawnEgPSQT[64] = {
        -6, -6, -6, -6, -6, -6, -6, -6,
        -11, -11, -8, -7, -7, -8, -11, -11,
        -12, -12, -11, -9, -9, -11, -12, -12,
        -19, -19, -11, -7, -7, -11, -19, -19,
        -19, -19, -11, -4, -4, -11, -19, -19,
        -16, -19, -11, 12, 7, -11, -15, -16,
        -19, -19, -11, -18, -18, -11, -19, -19,
        -6, -6, -6, -6, -6, -6, -6, -6,
};

constexpr Score knightMgPSQT[64] = {
        -10, -1, 0, 3, 1, 7, 0, -10,
        -7, 3, 4, 5, 5, 4, 3, -5,
        0, 2, 15, 15, 15, 15, 4, 0,
        8, 10, 15, 20, 20, 20, 11, 4,
        8, 10, 17, 23, 20, 21, 11, 8,
        8, 12, 15, 19, 19, 15, 12, 8,
        3, 11, 12, 13, 13, 12, 11, 3,
        -12, -7, -7, -7, -7, -7, -7, -12,
};

constexpr Score knightEgPSQT[64] = {
        -33, -12, -18, -4, -4, -10, -9, -34,
        -13, -17, -13, -1, -1, -20, -12, -17,
        -21, -6, -8, -9, -9, -9, -9, -18,
        -2, 2, -9, -4, -4, -5, -1, -3,
        -1, -1, 8, 16, 11, 8, 2, 0,
        0, 4, -3, 9, 10, 10, 4, 0,
        -5, 3, 4, 5, 5, 4, 3, -5,
        -20, -15, -15, -15, -15, -15, -15, -20,
};

constexpr Score bishopMgPSQT[64] = {
        -1, 2, -1, -2, 0, -2, 3, 10,
        -6, 8, 8, 8, 8, 8, 8, -2,
        -3, 7, 10, 10, 10, 10, 7, -3,
        -3, 7, 10, 9, 9, 10, 7, 7,
        1, 7, 10, 9, 9, 10, 7, 1,
        6, 8, 10, 9, 9, 10, 8, 7,
        -1, 24, 8, 8, 8, 8, 23, -2,
        -12, -8, -7, -7, -7, -7, -7, -12,
};

constexpr Score bishopEgPSQT[64] = {
        -13, -9, -17, -22, -20, -18, -16, -6,
        -19, -12, -11, -8, -12, -12, -12, -17,
        -19, -12, -9, -9, -9, -9, -12, -22,
        -17, -12, -9, -10, -10, -9, -12, -22,
        -20, -12, -9, -10, -10, -9, -12, -22,
        -5, -11, -9, -10, -10, -9, -11, -3,
        -18, 4, -11, -11, -9, -11, 7, -21,
        -22, -17, -17, -17, -17, -17, -17, -22,
};

constexpr Score rookMgPSQT[64] = {
        14, 12, 13, 12, 11, 15, 20, 13,
        10, 13, 13, 13, 13, 13, 13, 10,
        3, 8, 8, 8, 8, 8, 8, 3,
        3, 8, 8, 8, 8, 8, 8, 4,
        9, 12, 8, 8, 8, 14, 8, 9,
        11, 13, 5, 8, 10, 15, 14, 11,
        11, 16, 10, 7, 10, 15, 16, 11,
        16, 12, 8, 13, 13, 8, 16, 16,
};

constexpr Score rookEgPSQT[64] = {
        9, 7, 8, 8, 7, 8, 8, 9,
        -10, -7, -7, -7, -7, -7, -7, -10,
        -17, -12, -12, -12, -12, -12, -12, -17,
        -17, -12, -12, -12, -12, -12, -12, -17,
        -3, -12, -12, -12, -12, -12, -12, -16,
        3, -1, 4, -7, 1, -1, 4, 3,
        3, 6, 5, -7, 5, 7, 8, 3,
        8, 8, 7, -5, -5, 2, 8, 8,
};

constexpr Score queenMgPSQT[64] = {
        12, 12, 12, 12, 12, 12, 12, 12,
        15, 15, 15, 15, 12, 12, 12, 12,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 16, 16, 16, 16, 15, 15,
        15, 15, 16, 18, 18, 16, 15, 15,
        15, 15, 16, 18, 18, 16, 15, 15,
        15, 15, 17, 18, 18, 17, 15, 15,
        10, 10, 10, 10, 10, 10, 10, 10,
};

constexpr Score queenEgPSQT[64] = {
        -5, -5, -5, -5, -5, -5, -5, -5,
        -5, -5, -5, -5, -5, -5, -5, -5,
        -5, -5, -5, -5, -5, -5, -5, -5,
        -5, -5, -4, -4, -4, -4, -5, -5,
        -5, -5, -4, -2, -2, -4, -5, -5,
        -5, -5, -4, -2, -2, -4, -5, -5,
        -5, -5, -3, -2, -2, -3, -5, -5,
        -10, -10, -10, -10, -10, -10, -10, -10,
};


#endif //BLACKCORE_EVAL_H
