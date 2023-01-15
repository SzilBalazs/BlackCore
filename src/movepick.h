// BlackCore is a chess engine
// Copyright (c) 2023 SzilBalazs
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

#ifndef BLACKCORE_MOVEPICK_H
#define BLACKCORE_MOVEPICK_H

#include "movegen.h"
#include "threads.h"

enum SearchType {
    QSEARCH,
    MAIN_SEARCH,
    ROOT_SEARCH
};

enum PickStage {
    TT_MOVE,
    GENERATE_MOVES,
    PLAY_MOVES,
    FINISHED
};

template<SearchType searchType>
struct MovePicker {

    Move moveList[200];
    Score moveScores[200];
    Move *currMove = moveList;
    Move *lastMove;

    Move ttMove, pMove;
    bool skipTTMove = false;

    Position &pos;
    ThreadData &td;

    PickStage stage;

    inline MovePicker(Position &position, ThreadData &threadData, Move prevMove, Move hashMove) : pos(position), td(threadData) {
        ttMove = hashMove;
        pMove = prevMove;
        if (searchType == MAIN_SEARCH && hashMove.isOk()) {
            stage = TT_MOVE;
            skipTTMove = true;
        } else {
            stage = GENERATE_MOVES;
        }
    }

    inline void scoreMoves() {
        for (int i = 0; i < lastMove - moveList; i++) {
            if constexpr (searchType == ROOT_SEARCH)
                moveScores[i] = td.scoreRootNode(moveList[i]);
            else
                moveScores[i] = td.scoreMove(pos, pMove, moveList[i]);
        }
    }

    inline Move sortNextMove() {
        int best = currMove - moveList;
        for (int idx = best; idx < lastMove - moveList; idx++) {
            if (moveScores[idx] > moveScores[best]) {
                best = idx;
            }
        }
        std::swap(moveList[currMove - moveList], moveList[best]);
        std::swap(moveScores[currMove - moveList], moveScores[best]);
        return *currMove;
    }

    inline Move nextMove() {
        Move move;
        switch (stage) {
            case TT_MOVE:
                stage = GENERATE_MOVES;
                if (pos.isPseudoLegal(ttMove))
                    return ttMove;
            case GENERATE_MOVES:
                stage = PLAY_MOVES;
                if (searchType == QSEARCH) {
                    lastMove = generateMoves(pos, moveList, true);
                } else {
                    lastMove = generateMoves(pos, moveList, false);
                }
                scoreMoves();
            case PLAY_MOVES:
                move = sortNextMove();
                if (skipTTMove && move == ttMove) {
                    currMove++;
                    move = sortNextMove();
                }
                currMove++;
                if (currMove >= lastMove)
                    stage = FINISHED;
                return move;
            case FINISHED:
                return {};
        }
        return Move();
    }

    inline bool empty() const {
        return stage == FINISHED;
    }
};

#endif //BLACKCORE_MOVEPICK_H
