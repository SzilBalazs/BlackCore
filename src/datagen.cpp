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

#include "datagen.h"
#include "movegen.h"
#include "position.h"
#include "search.h"

#include <fstream>
#include <random>

struct DataEntry {
    std::string fen;
    Score score;
    int wdl;

    DataEntry(std::string _fen, Score _score) : fen(_fen), score(_score) {}
};

constexpr int RANDOM_PLY = 10;
constexpr int HASH_SIZE = 64;

void playGame(Position &pos, std::vector<DataEntry> &entries) {
    pos.loadPositionFromFen(STARTING_FEN);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1, 1000);
    int wdl, adjDraw = 0, adjWin = 0, ply = 0;

    for (int i = 0; i < RANDOM_PLY; i++) {
        Move moves[200];
        int cnt = generateMoves(pos, moves, false) - moves;
        if (cnt == 0) {
            return;
        } else {
            pos.makeMove(moves[dist(mt) % cnt]);
            ply++;
        }
    }

    SearchInfo info;
    info.maxDepth = 7;
    info.uciMode = false;

    while (ply <= 500) {
        if (pos.isRepetition() || pos.getMove50() >= 99) {
            wdl = 0;
            break;
        }

        Color stm = pos.getSideToMove();
        bool inCheck = bool(getAttackers(pos, pos.pieces<KING>(stm).lsb()));

        Move moves[200];
        int cnt = generateMoves(pos, moves, false) - moves;
        if (cnt == 0) {
            if (inCheck) {
                wdl = stm == WHITE ? 1 : -1;
            } else {
                wdl = 0;
            }
            break;
        }

        SearchResult result = startSearch(info, pos, 1);
        if (result.score == UNKNOWN_SCORE) {
            pos.display();
            out("This should never happen!");
            exit(1);
        }

        Score absScore = std::abs(result.score);

        if (absScore >= 1500) {
            adjWin++;
        } else {
            adjWin = 0;
        }

        if (absScore <= 10) {
            adjDraw++;
        } else {
            adjDraw = 0;
        }

        if (adjWin >= 5 || absScore > WORST_MATE) {
            wdl = result.score > 0 ? (stm == WHITE ? 1 : -1) : (stm == WHITE ? -1 : 1);
            break;
        }

        if (adjDraw >= 10) {
            wdl = 0;
            break;
        }

        if (result.bestMove.isQuiet() && !inCheck) {
            entries.emplace_back(pos.getFen(), result.score);
        }

        ply++;
        pos.makeMove(result.bestMove);
    }

    for (DataEntry &entry : entries) {
        entry.wdl = wdl;
    }
}

void startDataGen(int entryCount) {
    std::ofstream outFile;
    std::string outPath = "data_" + std::to_string(1) + ".plain";
    outFile.open(outPath, std::ios::app);

    initSearch();
    ttResize(HASH_SIZE);

    Position pos;
    int games = 0, finished = 0;

    std::vector<DataEntry> entries;
    while (finished < entryCount) {
        entries.clear();

        playGame(pos, entries);

        for (const DataEntry &entry : entries) {
            outFile << entry.fen << "<" << entry.score << ">" << entry.wdl << "\n";
        }

        finished += entries.size();
        games++;
    }

    outFile.close();
}