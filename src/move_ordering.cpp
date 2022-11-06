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

#include "move_ordering.h"
#include "tt.h"
#include "search.h"

#include <cstring>

constexpr Score winningCapture[6][6] = {
//    KING  PAWN    KNIGHT  BISHOP  ROOK    QUEEN
        {0, 0,      0,      0,      0,      0},       // KING
        {0, 800004, 800104, 800204, 800304, 800404},  // PAWN
        {0, 800003, 800103, 800203, 800303, 800403},  // KNIGHT
        {0, 800002, 800102, 800202, 800302, 800402},  // BISHOP
        {0, 800001, 800101, 800201, 800301, 800401},  // ROOK
        {0, 800000, 800100, 800200, 800300, 800400},  // QUEEN
};

constexpr Score losingCapture[6][6] = {
//    KING  PAWN    KNIGHT  BISHOP  ROOK    QUEEN
        {0, 0,      0,      0,      0,      0},       // KING
        {0, 200004, 200104, 200204, 200304, 200404},  // PAWN
        {0, 200003, 200103, 200203, 200303, 200403},  // KNIGHT
        {0, 200002, 200102, 200202, 200302, 200402},  // BISHOP
        {0, 200001, 200101, 200201, 200301, 200401},  // ROOK
        {0, 200000, 200100, 200200, 200300, 200400},  // QUEEN
};

Move killerMoves[MAX_PLY + 1][2];

// TODO Counter move history
Score historyTable[2][64][64];

void clearTables() {
    std::memset(killerMoves, 0, sizeof(killerMoves));
    std::memset(historyTable, 0, sizeof(historyTable));
}

void recordKillerMove(Move m, Ply ply) {
    killerMoves[ply][1] = killerMoves[ply][0];
    killerMoves[ply][0] = m;
}

void recordHHMove(Move move, Color color, Depth depth) {
    historyTable[color][move.getFrom()][move.getTo()] += depth * depth;
}

Score scoreQMove(const Position &pos, Move m) {
    if (m == getHashMove(pos.getHash())) {
        return 1000000;
    } else if (m.isPromo()) {
        if (m.isSpecial1() && m.isSpecial2()) { // Queen promo
            return 900000;
        } else { // Anything else, under promotions should only be played in really few cases
            return -100000;
        }
    } else {
        Score seeScore = see(pos, m);
        if (seeScore >= 0)
            return 800000 + seeScore;
        else
            return seeScore;
    }
}

Score scoreMove(const Position &pos, Move m, Ply ply) {
    if (m == getHashMove(pos.getHash())) {
        return 1000000;
    } else if (m.isPromo()) {
        if (m.isSpecial1() && m.isSpecial2()) { // Queen promo
            return 900000;
        } else { // Anything else, under promotions should only be played in really few cases
            return -100000;
        }
    } else if (m.isCapture()) {
        Square from = m.getFrom();
        Square to = m.getTo();

        if (see(pos, m) >= 0)
            return winningCapture[pos.pieceAt(from).type][pos.pieceAt(to).type];
        else
            return losingCapture[pos.pieceAt(from).type][pos.pieceAt(to).type];
    } else if (killerMoves[ply][0] == m) {
        return 750000;
    } else if (killerMoves[ply][1] == m) {
        return 700000;
    }
    return historyTable[pos.getSideToMove()][m.getFrom()][m.getTo()];
}