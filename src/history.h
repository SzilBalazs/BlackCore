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

    inline void reset() {
        std::memset(killerMoves, 0, sizeof(killerMoves));
    }

    inline void updateHistory(const Position &position, Move *quietMoves, int quiets, Move move, Ply ply, Score bonus) {

        Color stm = position.getSideToMove();

        if (move.isQuiet()) {
            killerMoves[ply][1] = killerMoves[ply][0];
            killerMoves[ply][0] = move;

            mainHistory[stm][move.getFrom()][move.getTo()] += bonus;

            for (int i = 0; i < quiets; i++) {
                mainHistory[stm][quietMoves[i].getFrom()][quietMoves[i].getTo()] -= bonus;
            }
        }
    }
};
