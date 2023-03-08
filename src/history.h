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

#include "move.h"
#include "position.h"

#include <cstring>

struct History {

    Move killerMoves[MAX_PLY + 1][2];
    Score mainHistory[2][64][64];
    Move counterMoves[64][64];

    inline void reset() {
        std::memset(killerMoves, 0, sizeof(killerMoves));
        std::memset(mainHistory, 0, sizeof(mainHistory));
    }

    inline void updateHistory(const Position &position, SearchStack *stack, Move *quietMoves, int quiets, Move move, Score bonus) {

        const Color stm = position.getSideToMove();
        const Square from = move.getFrom();
        const Square to = move.getTo();
        const Ply ply = stack->ply;
        const Move prevMove = (stack - 1)->move;

        if (move.isQuiet()) {
            killerMoves[ply][1] = killerMoves[ply][0];
            killerMoves[ply][0] = move;

            mainHistory[stm][from][to] += bonus;

            counterMoves[prevMove.getFrom()][prevMove.getTo()] = move;

            for (int i = 0; i < quiets; i++) {
                mainHistory[stm][quietMoves[i].getFrom()][quietMoves[i].getTo()] -= bonus;
            }
        }
    }
};
