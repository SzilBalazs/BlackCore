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

#include "history.h"
#include "position.h"
#include "timeman.h"
#include "uci.h"

#include <thread>

class SearchThread {

public:
    SearchThread();

    void startSearch();

    void startThread(const Position &pos, const SearchInfo &info);

    void stopThread();

    void joinThread();

    std::string getPvLine();

    int64_t getNodes() const;

    int64_t getNps() const;

private:
    Position position{};
    SearchInfo searchInfo{};
    TimeManager timeManager{};

    History history;

    Depth reductions[200][MAX_PLY + 1]{};

    Move pvArray[MAX_PLY + 1][MAX_PLY + 1]{};
    Ply pvLength[MAX_PLY + 1]{};

    int64_t nodes = 0;
    Ply selectivePly = 0;

    std::atomic_bool stop;
    std::thread thread;

    template<NodeType nodeType>
    Score search(SearchStack *stack, Depth depth, Score alpha, Score beta);

    template<NodeType nodeType>
    Score qsearch(SearchStack *stack, Score alpha, Score beta);
};

inline void init() {
    NNUE::init();
    initBitboard();
}
