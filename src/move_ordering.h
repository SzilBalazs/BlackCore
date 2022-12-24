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
extern Move counterMoves[64][64];
extern Score historyTable[2][64][64];

Score scoreMove(const Position &pos, Move prevMove, Move m, Ply ply);

Score scoreRootNode(Move m);

void clearTables();

void clearNodesSearchedTable();

void recordHistoryDifference(Color stm, Move move, Bitboard pieces);

void recordKillerMove(Move m, Ply ply);

void recordCounterMove(Move prevMove, Move move);

void recordHHMove(Move move, Color color, Score bonus);

void recordNodesSearched(Move m, U64 nodes);

#endif//BLACKCORE_MOVE_ORDERING_H
