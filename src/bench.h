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


#ifndef BLACKCORE_BENCH_H
#define BLACKCORE_BENCH_H

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

// Stores positions for perft test
struct TestPosition {
    std::string fen;
    Depth perftDepth;
    U64 perftResult;
};

const unsigned int posCount = 10;           // Number of test positions
const unsigned int benchPosCount = 20;      // Number of bench positions
const unsigned int searchTestHashSize = 32; // Transposition table size for benchmarking
const Depth SEARCH_DEPTH = 15;              // Depth used in benchmarks

const TestPosition testPositions[posCount] = {
        // Positions from CPW
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ", 6, 119060324},
        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 5, 193690690},
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 7, 178633661},
        {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 5, 15833292},
        {"r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ", 5, 15833292},
        {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 ", 5, 89941194},
        {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ", 5, 164075551},
        {"r3kb1r/1p3ppp/pqn1pn2/1Bpp1b2/3P1B2/1QP1PN2/PP1N1PPP/R3K2R w KQkq - 0 9", 5, 140824446},
        {"rnb1k2r/pppp1ppp/5q2/2b5/2BNP3/2N5/PPP2KPP/R1BQ3R w kq - 1 8", 5, 19782759},
        {"8/pp5p/8/2p2kp1/2Pp4/3P1KPP/PP6/8 w - - 0 32", 7, 13312960}};

const std::string benchPositions[benchPosCount] = {
        {"r1bq1k1r/pp3pp1/2nP4/7p/3p4/6N1/PPPQ1PPP/2KR1B1R b - - 1 16"},
        {"3Q4/1p3p2/2ppk3/4p2r/2PbP2p/3P3P/rq1BKP2/3R4 w - - 6 32"},
        {"8/4k3/4p3/1R3pp1/6p1/4PqP1/5P2/1R4K1 w - - 20 68"},
        {"8/kpq1n1p1/p3p3/8/N1p4P/P3P2K/1P3QP1/8 b - - 6 39"},
        {"8/5p2/2k5/7p/P1n2P2/1N4K1/8/8 b - - 1 50"},
        {"3r2k1/pp1b1pp1/1b6/2r3q1/3N4/P1BQPPPp/1P3K1P/3RR3 b - - 8 30"},
        {"1r4k1/p1q1r1p1/4p2p/pP1p4/3P4/R1P5/4Q1PP/R5K1 w - - 6 32"},
        {"8/5Q2/P6K/8/6q1/6k1/8/8 b - - 1 106"},
        {"rn1q1rk1/ppb1npp1/2p1p2p/3p3P/3PP3/2NQBP2/PPP2P2/R3KBR1 b Q - 5 11"},
        {"r2q1rk1/1p3pp1/p1npbn1p/4p3/4P3/P1BB1N2/P2Q1PPP/R3R1K1 b - - 2 14"},
        {"b1rr2k1/p3ppbp/6p1/N1n3q1/1p1N4/1P3P1P/4Q1P1/B2RR2K w - - 2 27"},
        {"4R3/3n4/1p1r1kp1/5p1p/p1r2N1P/5PK1/6P1/4R3 w - - 0 40"},
        {"8/8/4p3/p2kP1p1/1pN1p1P1/1P2K1P1/2P5/2b5 w - - 8 40"},
        {"r1bqk2r/pp1n1pp1/2pb3p/4p3/2PPB3/P3BN2/1PQ2PPP/R3K2R b KQkq - 1 12"},
        {"8/5p1k/2N4p/1p1pB2K/1P1P2P1/5P2/n1n5/8 w - - 3 45"},
        {"1qnr4/7p/1nk1p3/3b3Q/3P4/B2N2P1/5P1P/1R4K1 b - - 12 34"},
        {"1r6/5k2/8/8/5P1K/1P6/6N1/8 b - - 22 67"},
        {"r3k2r/pp1b1ppp/3b1n2/2np4/8/2N1PN2/Pq1BBPPP/R2QK2R w KQkq - 0 13"},
        {"8/3K4/4pk1p/7P/4P3/8/8/8 b - - 4 70"},
        {"8/2p3p1/3n4/1p6/3kpPB1/PP6/2PK1P2/8 w - - 5 45"}};

void testPerft();
void testSearch();

#endif //BLACKCORE_BENCH_H
