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

bool see(const Position &pos, Move move, Score threshold);

// clang-format off
constexpr int MVVLVA[6][6] = {
                              {0,  0,  0,  0,  0,  0},      // KING
                              {0, 14, 13, 12, 11, 10},      // PAWN
                              {0, 24, 23, 22, 21, 20},      // KNIGHT
                              {0, 34, 33, 32, 31, 30},      // BISHOP
                              {0, 44, 43, 42, 41, 40},      // ROOK
                              {0, 54, 53, 52, 51, 50}       // QUEEN
};
// clang-format on

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
    Score mainHistoryTable[2][64][64];
    Score counterHistoryTable[2][6][64][6][64];

    inline void clear() {
        selectiveDepth = 0;

        std::memset(pvArray, 0, sizeof(pvArray));
        std::memset(pvLength, 0, sizeof(pvLength));
        std::memset(killerMoves, 0, sizeof(killerMoves));
        std::memset(counterMoves, 0, sizeof(counterMoves));

        std::memset(counterHistoryTable, 0, sizeof(counterHistoryTable));

        for (Color color : {WHITE, BLACK}) {
            for (Square sq = A1; sq < 64; sq += 1) {
                for (Square sq2 = A1; sq2 < 64; sq2 += 1) {
                    mainHistoryTable[color][sq][sq2] /= 4;
                }
            }
        }
    }

    inline void reset() {
        nodes = 0;
        tbHits = 0;

        std::memset(mainHistoryTable, 0, sizeof(mainHistoryTable));

        mNodesSearched.lock();
        std::memset(nodesSearched, 0, sizeof(nodesSearched));
        mNodesSearched.unlock();

        clear();
    }

    void updateKillerMoves(Move m, Ply ply) {
        if (killerMoves[ply][0] != m) {
            killerMoves[ply][1] = killerMoves[ply][0];
            killerMoves[ply][0] = m;
        }
    }

    void updateCounterMoves(Move prevMove, Move move) {
        if (prevMove.isOk())
            counterMoves[prevMove.getFrom()][prevMove.getTo()] = move;
    }

    void updateMainHistory(Move move, Color color, Score bonus) {
        mainHistoryTable[color][move.getFrom()][move.getTo()] = std::clamp(mainHistoryTable[color][move.getFrom()][move.getTo()] + bonus, -30000, 30000);
    }

    void updateHistory(const Position &pos, Move move, SearchStack *stack, Move *quiets, int madeQuiets, Score bonus) {

        const Color stm = pos.getSideToMove();
        const Move prevMove = (stack - 1)->move;

        updateKillerMoves(move, stack->ply);
        updateCounterMoves(prevMove, move);

        if (prevMove.isOk()) {
            counterHistoryTable[pos.getSideToMove()][(stack - 1)->movedPiece.type][prevMove.getTo()][pos.pieceAt(move.getFrom()).type][move.getTo()] += bonus;

            for (int i = 0; i < madeQuiets; i++) {
                counterHistoryTable[pos.getSideToMove()][(stack - 1)->movedPiece.type][prevMove.getTo()][pos.pieceAt(quiets[i].getFrom()).type][quiets[i].getTo()] -= bonus;
            }
        }

        updateMainHistory(move, stm, bonus);

        for (int i = 0; i < madeQuiets; i++) {
            updateMainHistory(quiets[i], stm, -bonus);
        }
    }

    void updateNodesSearched(Move move, U64 totalNodes) {
        mNodesSearched.lock();
        nodesSearched[move.getFrom()][move.getTo()] += totalNodes;
        mNodesSearched.unlock();
    }

    Score getQuietHistory(const Position &pos, Move move, SearchStack *stack) {
        Score history = mainHistoryTable[pos.getSideToMove()][move.getFrom()][move.getTo()];

        const Move prevMove = (stack - 1)->move;
        if (prevMove.isOk())
            history += counterHistoryTable[pos.getSideToMove()][(stack - 1)->movedPiece.type][prevMove.getTo()][pos.pieceAt(move.getFrom()).type][move.getTo()];
        return history;
    }

    Score scoreRootNode(Move move) {
        return nodesSearched[move.getFrom()][move.getTo()] / 1000;
    }

    Score scoreMove(const Position &pos, Move move, SearchStack *stack) {
        const Ply ply = stack->ply;
        const Move prevMove = (stack - 1)->move;
        const Square from = move.getFrom();
        const Square to = move.getTo();

        if (move == getHashMove(pos.getHash())) {
            return 10000000;
        } else if (move.isPromo()) {
            if (move.isSpecial1() && move.isSpecial2()) { // Queen promotion
                return 9000000;
            } else { // Anything else, under promotions should only be played in really few cases
                return -3000000;
            }
        } else if (move.isCapture()) {
            bool good = see(pos, move, 0);
            return (good ? 8000000 : 2000000) + MVVLVA[move.equalFlag(EP_CAPTURE) ? PAWN : pos.pieceAt(to).type][pos.pieceAt(from).type];
        } else if (killerMoves[ply][0] == move) {
            return 7000000;
        } else if (killerMoves[ply][1] == move) {
            return 6000000;
        } else if (counterMoves[prevMove.getFrom()][prevMove.getTo()] == move) {
            return 5000000;
        }

        return getQuietHistory(pos, move, stack);
    }
};

#endif //BLACKCORE_THREADS_H
