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

#include "timeman.h"
#include <chrono>

unsigned int MOVE_OVERHEAD = 5;

constexpr U64 mask = 2047;

U64 startedSearch, idealTime, maxTime, maxNodes;

std::atomic<bool> stopped = true;

U64 getTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
}

void initTimeMan(U64 time, U64 inc, U64 movesToGo, U64 moveTime, U64 nodes) {

    startedSearch = getTime();
    stopped = false;

    maxNodes = nodes;

    if (moveTime != 0) {
        // We are limited how much can we search
        idealTime = moveTime - MOVE_OVERHEAD;
        maxTime = moveTime - MOVE_OVERHEAD;
    } else if (time == 0) {
        // We have infinite time
        idealTime = 0;
        maxTime = 0;
    } else {

        if (movesToGo == 0) {
            idealTime = 1 * inc + (time - MOVE_OVERHEAD) / 25;
            maxTime = 3 * inc + (time - MOVE_OVERHEAD) / 15;
        } else {
            idealTime = inc + (time - MOVE_OVERHEAD) / movesToGo;
            maxTime = inc + 5 * (time - MOVE_OVERHEAD) / (movesToGo + 10);
        }

        idealTime = std::min(idealTime, time - MOVE_OVERHEAD);
        maxTime = std::min(maxTime, time - MOVE_OVERHEAD);
    }
}

bool shouldEnd(U64 nodes, U64 totalNodes) {
    if ((nodes & mask) == 0 && !stopped) {
        stopped = (maxTime != 0 && getSearchTime() >= maxTime) || (maxNodes != 0 && totalNodes > maxNodes);
    }
    return stopped;
}

bool manageTime(double factor) {
    return getSearchTime() > std::min(U64(double(idealTime) * factor), maxTime) && maxTime != 0;
}

bool isInfiniteSearch() { return maxTime == 0; }

U64 getSearchTime() {
    return getTime() - startedSearch;
}

U64 getNps(U64 nodes) {
    U64 millis = getSearchTime();
    return millis == 0 ? 0 : nodes * 1000 / millis;
}
