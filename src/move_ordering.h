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

#ifndef BLACKCORE_MOVE_ORDERING_H
#define BLACKCORE_MOVE_ORDERING_H

#include "move.h"
#include "position.h"

extern Move killerMoves[MAX_PLY + 1][2];

Score scoreMove(const Position &pos, Move m, Ply ply);

Score scoreQMove(const Position &pos, Move m);

void clearTables();

void recordKillerMove(Move m, Ply ply);

void recordHHMove(Move move, Color color, Depth depth);

#endif //BLACKCORE_MOVE_ORDERING_H
