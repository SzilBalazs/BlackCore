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

#include "bench.h"
#include "search.h"
#include "timeman.h"
#include "tt.h"
#include <chrono>
#include <functional>
#include <iostream>
#include <string>

/*
 * Exits the program with exit code -1, if the movegen produces an
 * illegal move. Outputs a nodes per second value, which can be used to determine
 * the speed of the move generator.
 */
void testPerft() {

    // Initialize values
    initSearch();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    U64 totalNodes = 0;
    bool ok = true;

    // Iterating over the positions
    for (const TestPosition &tPos : testPositions) {

        Position pos = {tPos.fen};
        U64 nodes = perft<false>(pos, tPos.perftDepth);
        totalNodes += nodes;

        // If the node count doesn't match with the recorded value notify the user about it.
        if (nodes != tPos.perftResult) {
            ok = false;
            std::cout << tPos.fen << " failed! Result: " << nodes << " Expected: " << tPos.perftResult << std::endl;
        }
    }

    // Stopping the clock and calculating the nps
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    U64 elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    U64 nps = totalNodes * 1000 / elapsedTime;

    if (ok) {
        std::cout << "PERFT OK\n"
                  << totalNodes << " nodes " << nps << " nps" << std::endl;
    } else {
        std::cout << "PERFT FAILED" << std::endl;
        exit(1);
    }
}

// Outputs a node count for identifying the binary and a nodes per second,
// which shows the speed of the search.
void testSearch() {
    initSearch();
    ttResize(searchTestHashSize);

    U64 totalNodes = 0, nps = 0;

    for (const std::string &fen : benchPositions) {

        // Clear the transposition table for a deterministic behaviour.
        ttClear();

        Position pos = {fen};
        SearchInfo info;
        info.maxDepth = SEARCH_DEPTH;
        info.uciMode = false;

        startSearch(info, pos, 1);

        // Record the node count and the nps.
        totalNodes += getTotalNodes();
        nps += getNps(getTotalNodes());

        joinThreads(true);
    }

    std::cout << totalNodes << " nodes " << nps / benchPosCount << " nps" << std::endl;
}
