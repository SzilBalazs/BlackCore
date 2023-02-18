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

unsigned int MOVE_OVERHEAD = 20;

constexpr U64 mask = 1023;
constexpr long long INFINITE_LIMIT = LONG_LONG_MAX / 2;

bool minimalDepthReached;

long long startedSearch, idealTime, maxTime;
U64 maxNodes;

std::atomic<bool> stopped = true;

long long getTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
}

void initTimeManager(long long time, long long inc, long long movesToGo, long long moveTime, long long nodes) {

    startedSearch = getTime();
    stopped = false;
    minimalDepthReached = false;

    maxNodes = nodes == -1 ? INFINITE_LIMIT : nodes;

    if (moveTime != -1) {
        // We are limited how much can we search
        idealTime = moveTime - MOVE_OVERHEAD;
        maxTime = moveTime - MOVE_OVERHEAD;
    } else if (time == -1) {
        // We have infinite time
        idealTime = INFINITE_LIMIT;
        maxTime = INFINITE_LIMIT;
    } else {

        if (movesToGo == 0) {
            idealTime = 1 * inc + (time - MOVE_OVERHEAD) / 25;
            maxTime = 2 * inc + (time - MOVE_OVERHEAD) / 15;
        } else {
            idealTime = inc + (time - MOVE_OVERHEAD) / movesToGo;
            maxTime = 2 * idealTime;
        }
        idealTime = std::min(idealTime, time - MOVE_OVERHEAD);
        maxTime = std::min(maxTime, time - MOVE_OVERHEAD);
    }
}

bool shouldEnd(U64 nodes, U64 totalNodes) {
    if ((nodes & mask) == 0 && !stopped && minimalDepthReached && !isInfiniteSearch()) {
        stopped = getSearchTime() >= maxTime || totalNodes > maxNodes;
    }
    return stopped;
}

bool manageTime(double factor) {
    minimalDepthReached = true; // First time management is called at depth 5

    return getSearchTime() > std::min((long long) (double(idealTime) * factor), maxTime) && maxTime != INFINITE_LIMIT;
}

bool isInfiniteSearch() { return maxTime == INFINITE_LIMIT && maxNodes == INFINITE_LIMIT; }

long long getSearchTime() {
    return getTime() - startedSearch;
}

U64 getNps(U64 nodes) {
    U64 millis = getSearchTime();
    return millis == 0 ? 0 : nodes * 1000 / millis;
}
