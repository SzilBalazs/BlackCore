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
#include "search.h"
#include "tt.h"

#include <cstring>

Move killerMoves[MAX_PLY + 1][2];
Move counterMoves[64][64];
Score historyTable[2][64][64];
Bitboard historyDiff[2][64][64];
U64 nodesSearched[64][64];

void clearTables() {
    std::memset(killerMoves, 0, sizeof(killerMoves));
    std::memset(counterMoves, 0, sizeof(counterMoves));
    std::memset(historyTable, 0, sizeof(historyTable));
    std::memset(historyDiff, 0, sizeof(historyDiff));
}

void clearNodesSearchedTable() {
    std::memset(nodesSearched, 0, sizeof(nodesSearched));
}

int getHistoryDifference(Color stm, Move move, Bitboard pieces) {
    return (historyDiff[stm][move.getFrom()][move.getTo()] ^ pieces).popCount();
}

void recordHistoryDifference(Color stm, Move move, Bitboard pieces) {
    historyDiff[stm][move.getFrom()][move.getTo()] = pieces;
}

void recordKillerMove(Move m, Ply ply) {
    killerMoves[ply][1] = killerMoves[ply][0];
    killerMoves[ply][0] = m;
}

void recordCounterMove(Move prevMove, Move move) {
    counterMoves[prevMove.getFrom()][prevMove.getTo()] = move;
}

void recordHHMove(Move move, Color color, Score bonus) {
    historyTable[color][move.getFrom()][move.getTo()] += bonus;
    historyTable[color][move.getFrom()][move.getTo()] = std::max(0, historyTable[color][move.getFrom()][move.getTo()]);
}

void recordNodesSearched(Move m, U64 nodes) {
    nodesSearched[m.getFrom()][m.getTo()] = nodes;
}

Score scoreRootNode(Move m) {
    return nodesSearched[m.getFrom()][m.getTo()];
}

Score scoreMove(const Position &pos, Move prevMove, Move m, Ply ply) {
    if (m == getHashMove(pos.getHash())) {
        return 10000000;
    } else if (m.isPromo()) {
        if (m.isSpecial1() && m.isSpecial2()) {// Queen promo
            return 9000000;
        } else {// Anything else, under promotions should only be played in really few cases
            return -1000000;
        }
    } else if (m.isCapture()) {
        Score seeScore = see(pos, m);

        if (see(pos, m) >= 0)
            return 8000000 + seeScore;
        else
            return 2000000 + seeScore;
    } else if (counterMoves[prevMove.getFrom()][prevMove.getTo()] == m) {
        return 6000000;
    } else if (killerMoves[ply][0] == m) {
        return 5500000;
    } else if (killerMoves[ply][1] == m) {
        return 5000000;
    }
    Color stm = pos.getSideToMove();
    Bitboard occ = pos.occupied();
    int diff = getHistoryDifference(stm, m, occ);
    Score diffBonus = 0;
    if (diff == 0)
        diffBonus = 7500000;
    else if (diff == 1)
        diffBonus = 7400000;
    else if (diff == 2)
        diffBonus = 7300000;
    return diffBonus + historyTable[stm][m.getFrom()][m.getTo()];
}