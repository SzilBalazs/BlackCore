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

#include <string>
#include <chrono>
#include <iostream>
#include "bench.h"
#include "tt.h"
#include "search.h"
#include "movegen.h"
#include "timeman.h"

struct TestPosition {
    std::string fen;
    Depth perftDepth;
    U64 perftResult;
    Depth searchDepth;
};

const unsigned int posCount = 10;

const unsigned int searchTestHashSize = 8;

const TestPosition testPositions[posCount] = {
        // Positions from CPW
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ",                 6, 119060324, 8},
        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",         5, 193690690, 6},
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",                                     7, 178633661, 8},
        {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",          5, 15833292,  8},
        {"r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ",         5, 15833292,  8},
        {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 ",                5, 89941194,  8},
        {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ", 5, 164075551, 7},

        // Own positions
        {"r3kb1r/1p3ppp/pqn1pn2/1Bpp1b2/3P1B2/1QP1PN2/PP1N1PPP/R3K2R w KQkq - 0 9",   5, 140824446, 7},
        {"rnb1k2r/pppp1ppp/5q2/2b5/2BNP3/2N5/PPP2KPP/R1BQ3R w kq - 1 8",              5, 19782759,  7},
        {"8/pp5p/8/2p2kp1/2Pp4/3P1KPP/PP6/8 w - - 0 32",                              7, 13312960,  8}
};

U64 perft(Position &position, Depth depth) {
    Move moves[200];
    Move *movesEnd = generateMoves(position, moves, false);
    if (depth == 1) return movesEnd - moves;
    U64 nodes = 0;
    for (Move *it = moves; it != movesEnd; it++) {
        position.makeMove(*it);
        nodes += perft(position, depth - 1);
        position.undoMove(*it);
    }
    return nodes;
}

void testPerft() {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    U64 totalNodes = 0;
    bool ok = true;
    for (const TestPosition &tPos : testPositions) {
        Position pos = {tPos.fen};
        U64 nodes = perft(pos, tPos.perftDepth);
        totalNodes += nodes;
        if (nodes != tPos.perftResult) {
            ok = false;
            std::cout << tPos.fen << " failed! Result: " << nodes << " Expected: " << tPos.perftResult << std::endl;
        }
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    U64 elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    U64 nps = totalNodes * 1000 / elapsedTime;

    if (ok) {
        std::cout << "PERFT OK\n" << totalNodes << " nodes " << nps << " nps" << std::endl;
    } else {
        std::cout << "PERFT FAILED" << std::endl;
    }
}

void testSearch() {
    ttResize(searchTestHashSize);

    startSearch(0, 0, 0, 0);
    for (const TestPosition &tPos : testPositions) {
        ttClear();
        Position pos = {tPos.fen};
        iterativeDeepening(pos, tPos.searchDepth, false);
    }

    std::cout << nodeCount << " nodes " << getNps() << " nps" << std::endl;
}
