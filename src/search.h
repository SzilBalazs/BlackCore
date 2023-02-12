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

#ifndef BLACKCORE_SEARCH_H
#define BLACKCORE_SEARCH_H

#include "movegen.h"
#include "tune.h"
#include "uci.h"
#include <atomic>

U64 getTotalNodes();

void initLmr();

// Initializes stuff that is needed for a search.
inline void initSearch() {
    initBitboard();
    initLmr();
    NNUE::init();
}

bool see(const Position &pos, Move move, Score threshold);

void joinThreads(bool waitToFinish);

void startSearch(SearchInfo &searchInfo, Position &pos, int threadCount);

#endif //BLACKCORE_SEARCH_H
