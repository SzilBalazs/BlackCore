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

#include "eval.h"

constexpr Score wPawnTable[64] = {0, 0, 0, 0, 0, 0, 0, 0,
                                  10, 10, 10, -30, -30, 10, 10, 10,
                                  10, 10, -5, 0, 0, -5, 10, 10,
                                  0, 0, 0, 30, 30, 0, 0, 0,
                                  5, 5, 5, 20, 20, 5, 5, 5,
                                  5, 5, 15, 15, 15, 15, 5, 5,
                                  50, 50, 50, 50, 50, 50, 50, 50,
                                  0, 0, 0, 0, 0, 0, 0, 0};

constexpr Score bPawnTable[64] = {0, 0, 0, 0, 0, 0, 0, 0,
                                  50, 50, 50, 50, 50, 50, 50, 50,
                                  5, 5, 15, 15, 15, 15, 5, 5,
                                  5, 5, 5, 20, 20, 5, 5, 5,
                                  0, 0, 0, 30, 30, 0, 0, 0,
                                  10, 10, -5, 0, 0, -5, 10, 10,
                                  10, 10, 10, -30, -30, 10, 10, 10,
                                  0, 0, 0, 0, 0, 0, 0, 0};

constexpr Score knightTable[64] = {-50, -40, -30, -30, -30, -30, -40, -50,
                                   -40, -20, 5, 5, 5, 5, -20, -40,
                                   -30, 5, 10, 15, 15, 10, 5, -30,
                                   -30, 5, 15, 20, 20, 15, 5, -30,
                                   -30, 5, 15, 20, 20, 15, 5, -30,
                                   -30, 5, 10, 15, 15, 10, 5, -30,
                                   -40, -20, 5, 5, 5, 5, -20, -40,
                                   -50, -40, -30, -30, -30, -30, -40, -50};

constexpr Score wBishopTable[64] = {-20, -10, -15, -10, -10, -15, -10, -20,
                                    -5, 5, 0, 7, 7, 0, 5, -5,
                                    -5, 0, 0, 0, 0, 0, 0, -5,
                                    -5, 0, 15, 10, 10, 15, 0, -5,
                                    -5, 15, 0, 5, 5, 0, 15, -5,
                                    -5, 0, 0, 0, 0, 0, 0, -5,
                                    -5, 0, 0, 0, 0, 0, 0, -5,
                                    -20, -10, -10, -10, -10, -10, -10, -20};

constexpr Score bBishopTable[64] = {-20, -10, -10, -10, -10, -10, -10, -20,
                                    -5, 0, 0, 0, 0, 0, 0, -5,
                                    -5, 0, 0, 0, 0, 0, 0, -5,
                                    -5, 15, 0, 5, 5, 0, 15, -5,
                                    -5, 0, 15, 10, 10, 15, 0, -5,
                                    -5, 0, 0, 0, 0, 0, 0, -5,
                                    -5, 5, 0, 7, 7, 0, 5, -5,
                                    -20, -10, -15, -10, -10, -15, -10, -20};

constexpr Score wRookTable[64] = {-5, 0, 0, 0, 0, 0, 0, -5,
                                  -5, 0, 0, 0, 0, 0, 0, -5,
                                  -5, 0, 0, 0, 0, 0, 0, -5,
                                  -5, 0, 0, 0, 0, 0, 0, -5,
                                  -5, 0, 0, 0, 0, 0, 0, -5,
                                  -5, 0, 0, 0, 0, 0, 0, -5,
                                  5, 15, 15, 15, 15, 15, 15, 5,
                                  0, 0, 0, 0, 0, 0, 0, 0};

constexpr Score bRookTable[64] = {0, 0, 0, 0, 0, 0, 0, 0,
                                  5, 15, 15, 15, 15, 15, 15, 5,
                                  -5, 0, 0, 0, 0, 0, 0, -5,
                                  -5, 0, 0, 0, 0, 0, 0, -5,
                                  -5, 0, 0, 0, 0, 0, 0, -5,
                                  -5, 0, 0, 0, 0, 0, 0, -5,
                                  -5, 0, 0, 0, 0, 0, 0, -5,
                                  -5, 0, 0, 0, 0, 0, 0, -5};

constexpr Score wKingTable[64] = {30, 30, 10, 0, 0, 10, 30, 30,
                                  20, 20, 0, 0, 0, 0, 20, 20,
                                  -10, -20, -20, -20, -20, -20, -20, -10,
                                  -20, -25, -30, -40, -40, -30, -25, -20,
                                  -30, -40, -40, -40, -40, -40, -40, -30,
                                  -40, -40, -40, -40, -40, -40, -40, -40,
                                  -40, -50, -50, -50, -50, -50, -50, -40,
                                  -60, -60, -60, -60, -60, -60, -60, -60};

constexpr Score bKingTable[64] = {-60, -60, -60, -60, -60, -60, -60, -60,
                                  -40, -50, -50, -50, -50, -50, -50, -40,
                                  -40, -40, -40, -40, -40, -40, -40, -40,
                                  -30, -40, -40, -40, -40, -40, -40, -30,
                                  -20, -25, -30, -40, -40, -30, -25, -20,
                                  -10, -20, -20, -20, -20, -20, -20, -10,
                                  20, 20, 0, 0, 0, 0, 20, 20,
                                  30, 30, 10, 0, 0, 10, 30, 30};

Score eval(const Position &pos) {

    Bitboard pawns = pos.pieces<PAWN>();

    Score whiteEval = 0;

    Square wKing = pos.pieces<WHITE, KING>().lsb();
    Bitboard wPawns = pos.pieces<WHITE, PAWN>();
    Bitboard wKnights = pos.pieces<WHITE, KNIGHT>();
    Bitboard wBishops = pos.pieces<WHITE, BISHOP>();
    Bitboard wRooks = pos.pieces<WHITE, ROOK>();
    Bitboard wQueens = pos.pieces<WHITE, QUEEN>();

    whiteEval += wPawns.popCount() * PIECE_VALUES[PAWN];
    whiteEval += wKnights.popCount() * PIECE_VALUES[KNIGHT];
    whiteEval += wBishops.popCount() * PIECE_VALUES[BISHOP];
    whiteEval += wRooks.popCount() * PIECE_VALUES[ROOK];
    whiteEval += wQueens.popCount() * PIECE_VALUES[QUEEN];
    whiteEval += wKingTable[wKing];

    while (wPawns) {
        whiteEval += wPawnTable[wPawns.popLsb()];
    }

    while (wKnights) {
        whiteEval += knightTable[wKnights.popLsb()];
    }

    while (wBishops) {
        whiteEval += wBishopTable[wBishops.popLsb()];
    }

    while (wRooks) {
        Square square = wRooks.popLsb();
        whiteEval += wRookTable[square];

        unsigned int pawnCntOnFile = (rookMask(square) & pawns).popCount();

        if (pawnCntOnFile == 0) {
            whiteEval += ROOK_OPEN_BONUS;
        } else if (pawnCntOnFile == 1) {
            whiteEval += ROOK_HOPEN_BONUS;
        }
    }

    Score blackEval = 0;

    Square bKing = pos.pieces<BLACK, KING>().lsb();
    Bitboard bPawns = pos.pieces<BLACK, PAWN>();
    Bitboard bKnights = pos.pieces<BLACK, KNIGHT>();
    Bitboard bBishops = pos.pieces<BLACK, BISHOP>();
    Bitboard bRooks = pos.pieces<BLACK, ROOK>();
    Bitboard bQueens = pos.pieces<BLACK, QUEEN>();

    blackEval += bPawns.popCount() * PIECE_VALUES[PAWN];
    blackEval += bKnights.popCount() * PIECE_VALUES[KNIGHT];
    blackEval += bBishops.popCount() * PIECE_VALUES[BISHOP];
    blackEval += bRooks.popCount() * PIECE_VALUES[ROOK];
    blackEval += bQueens.popCount() * PIECE_VALUES[QUEEN];
    blackEval += bKingTable[bKing];

    while (bPawns) {
        blackEval += bPawnTable[bPawns.popLsb()];
    }

    while (bKnights) {
        blackEval += knightTable[bKnights.popLsb()];
    }

    while (bBishops) {
        blackEval += bBishopTable[bBishops.popLsb()];
    }

    while (bRooks) {
        Square square = bRooks.popLsb();
        blackEval += bRookTable[square];

        unsigned int pawnCntOnFile = (rookMask(square) & pawns).popCount();

        if (pawnCntOnFile == 0) {
            blackEval += ROOK_OPEN_BONUS;
        } else if (pawnCntOnFile == 1) {
            blackEval += ROOK_HOPEN_BONUS;
        }
    }

    if (pos.getSideToMove() == WHITE)
        return whiteEval - blackEval + TEMPO_SCORE;
    else
        return blackEval - whiteEval + TEMPO_SCORE;
}