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

#pragma once

#include "history.h"
#include "movegen.h"
#include "position.h"
#include "search.h"

enum ListType {
    LIST_AB,
    LIST_Q,
    LIST_ROOT
};

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

template<ListType type>
class MoveList {

public:
    // Constructor that generates and scores legal moves.
    inline MoveList(const Position &position, const History &history, SearchStack *stack, Move hashMove) {
        count = generateMoves(position, moves, (type == LIST_Q)) - moves;
        Ply ply = stack->ply;

        for (int i = 0; i < count; i++) {

            if (moves[i] == hashMove) {
                scores[i] = 10'000'000;
            } else if (moves[i].isCapture()) {
                PieceType moved = position.pieceAt(moves[i].getFrom()).type;
                PieceType captured = moves[i].equalFlag(EP_CAPTURE) ? PAWN : position.pieceAt(moves[i].getTo()).type;
                scores[i] = 8'000'000 + MVVLVA[captured][moved];
            } else if (moves[i] == history.killerMoves[ply][0]) {
                scores[i] = 7'000'000;
            } else if (moves[i] == history.killerMoves[ply][1]) {
                scores[i] = 6'000'000;
            } else {
                scores[i] = 0;
            }
        }
    }

    // Returns true if there are no more moves left.
    inline bool empty() const {
        return index == count;
    }

    // Sorts and returns the next best scored move.
    inline Move nextMove() {
        int best = index;
        for (int i = index + 1; i < count; i++) {
            if (scores[i] > scores[best]) {
                best = i;
            }
        }
        std::swap(moves[index], moves[best]);
        std::swap(scores[index], scores[best]);

        return moves[index++];
    }

private:
    Move moves[200];
    Score scores[200];
    int count, index = 0;
};