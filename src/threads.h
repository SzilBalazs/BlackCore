// BlackCore is a chess engine
// Copyright (c) 2022-2023 SzilBalazs
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef BLACKCORE_THREADS_H
#define BLACKCORE_THREADS_H

#include "search.h"
#include "tt.h"

#include <algorithm>
#include <cstring>
#include <mutex>

extern std::mutex mNodesSearched;
extern U64 nodesSearched[64][64];

Score see(const Position &pos, Move move);

struct ThreadData {

    int threadId;
    Position position;

    U64 nodes = 0;
    Depth selectiveDepth = 0;
    U64 tbHits = 0;

    bool uciMode = false;

    // Arrays used for retrieving the principal variation.
    Move pvArray[MAX_PLY + 1][MAX_PLY + 1];
    Ply pvLength[MAX_PLY + 1];

    // Arrays used for move ordering.
    Move killerMoves[MAX_PLY + 1][2];
    Move counterMoves[64][64];
    Score hhTable[2][64][64];
    Score chTable[6][64][64][64];

    inline void clear() {
        selectiveDepth = 0;

        std::memset(pvArray, 0, sizeof(pvArray));
        std::memset(pvLength, 0, sizeof(pvLength));
        std::memset(killerMoves, 0, sizeof(killerMoves));
        std::memset(counterMoves, 0, sizeof(counterMoves));

        for (Color color : {WHITE, BLACK}) {
            for (Square sq = A1; sq < 64; sq += 1) {
                for (Square sq2 = A1; sq2 < 64; sq2 += 1) {
                    hhTable[color][sq][sq2] /= 4;
                }
            }
        }

        for (int type = 0; type < 6; type++) {
            for (Square sq = A1; sq < 64; sq += 1) {
                for (Square sq2 = A1; sq2 < 64; sq2 += 1) {
                    for (Square sq3 = A1; sq3 < 64; sq3 += 1) {
                        chTable[type][sq][sq2][sq3] /= 4;
                    }
                }
            }
        }
    }

    inline void reset() {
        nodes = 0;
        tbHits = 0;

        std::memset(hhTable, 0, sizeof(hhTable));
        std::memset(chTable, 0, sizeof(chTable));

        mNodesSearched.lock();
        std::memset(nodesSearched, 0, sizeof(nodesSearched));
        mNodesSearched.unlock();

        clear();
    }

    void updateHistory(SearchStack *stack, const std::vector<Move> &quiets, Score hhBonus) {

        const Move move = stack->move;
        const Move counterMove = (stack - 1)->move;
        const Color stm = position.getSideToMove();

        killerMoves[stack->ply][1] = killerMoves[stack->ply][0];
        killerMoves[stack->ply][0] = move;

        counterMoves[counterMove.getFrom()][counterMove.getTo()] = move;

        // TODO Clamp HH values
        hhTable[stm][move.getFrom()][move.getTo()] += hhBonus;
        chTable[(stack - 1)->movedPiece.type][counterMove.getTo()][move.getFrom()][move.getTo()] += hhBonus;

        for (Move m : quiets) {
            hhTable[stm][m.getFrom()][m.getTo()] -= hhBonus;
            chTable[(stack - 1)->movedPiece.type][counterMove.getTo()][m.getFrom()][m.getTo()] -= hhBonus;
        }
    }

    Score getHistory(SearchStack *stack) {

        const Move move = stack->move;
        const Move counterMove = (stack - 1)->move;
        const Color stm = position.getSideToMove();

        Score hhScore = hhTable[stm][move.getFrom()][move.getTo()];
        Score chScore = chTable[(stack - 1)->movedPiece.type][counterMove.getTo()][move.getFrom()][move.getTo()];

        return hhScore;
    }

    void updateNodesSearched(Move move, U64 totalNodes) {
        mNodesSearched.lock();
        nodesSearched[move.getFrom()][move.getTo()] += totalNodes;
        mNodesSearched.unlock();
    }

    Score scoreRootNode(Move move) {
        return nodesSearched[move.getFrom()][move.getTo()] / 1000;
    }

    Score scoreMove(SearchStack *stack, Move move) {
        const Move counterMove = (stack - 1)->move;

        if (move == getHashMove(position.getHash())) {
            return 10000000;
        } else if (move.isPromo()) {
            if (move.isSpecial1() && move.isSpecial2()) { // Queen promotion
                return 9000000;
            } else { // Anything else, under promotions should only be played in really few cases
                return -3000000;
            }
        } else if (move.isCapture()) {
            Score seeScore = see(position, move);

            if (see(position, move) >= 0)
                return 8000000 + seeScore;
            else
                return 2000000 + seeScore;
        } else if (killerMoves[stack->ply][0] == move) {
            return 7000000;
        } else if (killerMoves[stack->ply][1] == move) {
            return 6000000;
        } else if (counterMoves[counterMove.getFrom()][counterMove.getTo()] == move) {
            return 5000000;
        }

        return getHistory(stack);
    }
};

#endif //BLACKCORE_THREADS_H
