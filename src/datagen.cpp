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
#include "egtb.h"
#include "movegen.h"
#include "position.h"
#include "search.h"

#include <fstream>
#include <random>
#include <utility>

struct DataEntry {
    std::string fen;
    Score score;
    int wdl;

    Color stm;

    DataEntry(std::string _fen, Score _score, Color _stm) : fen(std::move(_fen)), score(_score), stm(_stm) {}
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
            entries.emplace_back(pos.getFen(), result.score, pos.getSideToMove());
        }

        ply++;
        pos.makeMove(result.bestMove);
        if (result.bestMove.isCapture() || pos.pieceAt(result.bestMove.getTo()).type == PAWN)
            pos.resetStack();

        unsigned int tbResult = TBProbe(pos);
        if (tbResult != TB_RESULT_FAILED) {
            if (tbResult == TB_WIN) {
                wdl = stm == WHITE ? 1 : -1;
            } else if (tbResult == TB_LOSS) {
                wdl = stm == BLACK ? 1 : -1;
            } else {
                wdl = 0;
            }

            break;
        }
    }

    for (DataEntry &entry : entries) {
        if (wdl == 1) {
            entry.wdl = entry.stm == WHITE ? 1 : -1;
        } else if (wdl == -1) {
            entry.wdl = entry.stm == BLACK ? 1 : -1;
        } else {
            entry.wdl = 0;
        }
    }
}

void startDataGen(int entryCount, int id) {
    std::ofstream outFile;
    std::string outPath = "data/data" + std::to_string(id) + ".plain";
    outFile.open(outPath, std::ios::app);

    tb_init("tb/");

    initSearch();
    ttResize(HASH_SIZE);

    Position pos;
    int games = 0, finished = 0;

    std::vector<DataEntry> entries;

    auto start = std::chrono::system_clock::now();
    while (finished < entryCount) {
        entries.clear();

        playGame(pos, entries);

        for (const DataEntry &entry : entries) {
            outFile << entry.fen << "<" << entry.score << ">" << entry.wdl << "\n";
        }

        if (games % 50 == 0) {
            auto now = std::chrono::system_clock::now();
            auto t = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
            std::cout << "[" << finished << "/" << entryCount << "] " << (finished / (t + 1)) << " fen/sec" << std::endl;
        }

        finished += entries.size();
        games++;
    }

    outFile.close();
}