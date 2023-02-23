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
#include "position.h"
#include "search.h"

#include <fstream>

struct DataEntry {
    std::string fen;
    Score score;
    int wdl;

    DataEntry(std::string _fen, Score _score, int _wdl) : fen(_fen), score(_score), wdl(_wdl) {}
};

void playGame(Position &pos, std::vector<DataEntry> &entries) {
    pos.loadPositionFromFen(STARTING_FEN);

    entries.emplace_back(pos.getFen(), -10, 10);
}

void startDataGen(int entryCount) {
    std::ofstream outFile;
    std::string outPath = "data_" + std::to_string(1) + ".plain";
    outFile.open(outPath, std::ios::app);

    initSearch();

    Position pos;
    int games = 0, finished = 0;

    std::vector<DataEntry> entries;
    while (finished < entryCount) {
        out(finished);
        entries.clear();

        playGame(pos, entries);

        finished += entries.size();
        games++;
    }

    outFile.close();
}