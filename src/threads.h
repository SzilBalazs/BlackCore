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

#ifndef BLACKCORE_THREADS_H
#define BLACKCORE_THREADS_H

#include "move.h"
#include "tt.h"

#include <cstring>
#include <mutex>

const int HISTORY_DIFF_SLOTS = 4;

extern std::mutex mNodesSearched;
extern U64 nodesSearched[64][64];

Score see(const Position &pos, Move move);

struct ThreadData {

    int threadId;
    Position position;

    U64 nodes = 0;
    Depth selectiveDepth = 0;

    bool uciMode = false;

    Move pvArray[MAX_PLY + 1][MAX_PLY + 1];
    int pvLength[MAX_PLY + 1];

    // Move ordering
    Move killerMoves[MAX_PLY + 1][2];
    Move counterMoves[64][64];
    Score historyTable[2][64][64];
    Bitboard historyDiff[2][64][64][HISTORY_DIFF_SLOTS];
    int historyDiffReplace[2][64][64];

    inline void clear() {
        selectiveDepth = 0;

        std::memset(pvArray, 0, sizeof(pvArray));
        std::memset(pvLength, 0, sizeof(pvLength));
        std::memset(killerMoves, 0, sizeof(killerMoves));
        std::memset(counterMoves, 0, sizeof(counterMoves));
        std::memset(historyTable, 0, sizeof(historyTable));
        std::memset(historyDiffReplace, 0, sizeof(historyDiffReplace));
        std::memset(historyDiff, 0, sizeof(historyDiff));
    }

    inline void reset() {
        nodes = 0;

        mNodesSearched.lock();
        std::memset(nodesSearched, 0, sizeof(nodesSearched));
        mNodesSearched.unlock();

        clear();
    }

    int getHistoryDifference(Color stm, Move move, Bitboard occ) {
        int diff = 100;
        for (int idx = 0; idx < HISTORY_DIFF_SLOTS; idx++) {
            diff = std::min(diff, (historyDiff[stm][move.getFrom()][move.getTo()][idx] ^ occ).popCount());
        }
        return diff;
    }

    void updateHistoryDifference(Color stm, Move move, Bitboard pieces) {
        Square from = move.getFrom();
        Square to = move.getTo();
        historyDiff[stm][from][to][historyDiffReplace[stm][from][to]] = pieces;
        historyDiffReplace[stm][from][to]++;
        historyDiffReplace[stm][from][to] %= HISTORY_DIFF_SLOTS;
    }

    void updateKillerMoves(Move m, Ply ply) {
        killerMoves[ply][1] = killerMoves[ply][0];
        killerMoves[ply][0] = m;
    }

    void updateCounterMoves(Move prevMove, Move move) {
        counterMoves[prevMove.getFrom()][prevMove.getTo()] = move;
    }

    void updateHH(Move move, Color color, Score bonus) {
        historyTable[color][move.getFrom()][move.getTo()] += bonus;
    }

    void updateNodesSearched(Move move, U64 totalNodes) {
        mNodesSearched.lock();
        nodesSearched[move.getFrom()][move.getTo()] += totalNodes;
        mNodesSearched.unlock();
    }

    Score scoreRootNode(Move move) {
        return nodesSearched[move.getFrom()][move.getTo()] / 1000;
    }

    Score scoreMove(const Position &pos, Move prevMove, Move move) {

        if (move == getHashMove(pos.getHash())) {
            return 10000000;
        } else if (move.isPromo()) {
            if (move.isSpecial1() && move.isSpecial2()) { // Queen promo
                return 9000000;
            } else { // Anything else, under promotions should only be played in really few cases
                return -3000000;
            }
        } else if (move.isCapture()) {
            Score seeScore = see(pos, move);

            if (see(pos, move) >= 0)
                return 8000000 + seeScore;
            else
                return 2000000 + seeScore;
        } else if (counterMoves[prevMove.getFrom()][prevMove.getTo()] == move) {
            return 5000000;
        }
        Color stm = pos.getSideToMove();
        Bitboard occ = pos.occupied();
        int diff = getHistoryDifference(stm, move, occ);
        Score diffBonus = 0;
        if (diff == 0)
            diffBonus = 5600000;
        else if (diff == 1)
            diffBonus = 5500000;
        else if (diff == 2)
            diffBonus = 5400000;
        return diffBonus + historyTable[stm][move.getFrom()][move.getTo()];
    }
};

#endif //BLACKCORE_THREADS_H
