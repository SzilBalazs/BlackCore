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


#ifndef BLACKCORE_BENCH_H
#define BLACKCORE_BENCH_H

#include "movegen.h"

#include <iostream>

// Function that outputs the number of positions which can be got into,
// in depth number of legal moves. Used for validating the move generator.
// For more information: https://www.chessprogramming.org/Perft
template<bool output>
U64 perft(Position &position, Depth depth) {

    Move moves[200];
    Move *movesEnd = generateMoves(position, moves, false);

    // Bulk counting the number of moves at depth 1.
    if (depth == 1)
        return movesEnd - moves;

    // DFS like routine, calling itself recursively with lowered depth.
    U64 nodes = 0;
    for (Move *it = moves; it != movesEnd; it++) {
        position.makeMove(*it);
        U64 a = perft<false>(position, depth - 1);
        if constexpr (output) {
            std::cout << *it << ": " << a << std::endl;
        }
        nodes += a;
        position.undoMove(*it);
    }
    return nodes;
}

void testPerft();

void testSearch();

#endif//BLACKCORE_BENCH_H
