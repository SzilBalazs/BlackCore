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

#include "timeman.h"
#include "position.h"
#include <chrono>

unsigned int MOVE_OVERHEAD = 10;

constexpr U64 mask = 1023;

U64 startedSearch, shouldSearch, searchTime, maxSearch, stabilityTime, maxNodes;

std::atomic<bool> stopped = true;

U64 getTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
}

void initTimeMan(U64 time, U64 inc, U64 movesToGo, U64 moveTime, U64 nodes) {
    nodeCount = 0;

    movesToGo = movesToGo == 0 ? 20 : movesToGo + 1;

    startedSearch = getTime();
    stabilityTime = 0;
    stopped = false;

    maxNodes = nodes;

    if (moveTime != 0) {
        // We are limited how much can we search
        shouldSearch = moveTime - MOVE_OVERHEAD;
        maxSearch = moveTime - MOVE_OVERHEAD;
    } else if (time == 0) {
        // We have infinite time
        shouldSearch = 0;
        maxSearch = 0;
    } else {

        U64 panicTime = time / (movesToGo + 10) + 2 * inc;
        stabilityTime = time / 500;

        shouldSearch = std::min(time - MOVE_OVERHEAD, time / movesToGo + inc * 3 / 4 - MOVE_OVERHEAD);
        maxSearch = std::min(time - MOVE_OVERHEAD, shouldSearch + panicTime);
    }

    searchTime = shouldSearch;
}

void allocateTime(int stability) {
    U64 newSearchTime = shouldSearch - stability * stabilityTime;
    searchTime = std::min(maxSearch, newSearchTime);
}

bool shouldEnd() {
    if ((nodeCount & mask) == 0 && !stopped) {
        stopped = (maxSearch != 0 && getSearchTime() >= searchTime) || (maxNodes != 0 && nodeCount > maxNodes);
    }
    return stopped;
}

U64 getSearchTime() {
    return getTime() - startedSearch;
}

U64 getNps() {
    U64 millis = getSearchTime();
    return millis == 0 ? 0 : nodeCount * 1000 / millis;
}
