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

#pragma once

#include "movegen.h"

#include <iostream>

/*
 * Outputs the number of positions which can be got into,
 * in depth number of legal moves. Used for validating the move generator.
 * For more information: https://www.chessprogramming.org/Perft
 */
template<bool output>
U64 perft(Position &position, Depth depth) {

    Move moves[200]; // Stores the legal moves in the position
    Move *movesEnd = generateMoves(position, moves, false);

    // Bulk counting the number of moves at depth 1.
    if (depth == 1)
        return movesEnd - moves;

    // DFS like routine, calling itself recursively with lowered depth.
    U64 nodes = 0;
    for (Move *it = moves; it != movesEnd; it++) {
        position.makeMove(*it);
        U64 nodeCount = perft<false>(position, depth - 1);
        if constexpr (output) {
            std::cout << *it << ": " << nodeCount << std::endl; // Used for debugging purposes.
        }
        nodes += nodeCount;
        position.undoMove(*it);
    }
    return nodes;
}


void testPerft();
void testSearch(U64 expectedResult);
