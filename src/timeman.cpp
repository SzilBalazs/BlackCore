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
#include "uci.h"

#include <chrono>

constexpr U64 mask = (1ULL << 15) - 1;
U64 searchStartedAt = 0;
U64 searchShouldEnd = 0;
bool stop = false;

U64 getTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
}

void startSearch(U64 time, U64 inc, U64 movestogo, U64 movetime) {
    nodeCount = 0;
    searchStartedAt = getTime();
    stop = false;
    if (time == 0 || movetime != 0) {
        searchShouldEnd = searchStartedAt + movetime;
    } else {
        searchShouldEnd = searchStartedAt + inc + (time / (movestogo + 5));
    }
}

void stopSearch() {
    stop = true;
}

bool shouldEnd() {
    if ((nodeCount & mask) == 0 && !stop) {
        stop = searchShouldEnd != searchStartedAt && searchShouldEnd <= getTime();
    }
    return stop;
}

U64 getSearchTime() {
    return getTime() - searchStartedAt;
}

U64 getNps() {
    U64 millis = getSearchTime();
    return millis == 0 ? 0 : nodeCount * 1000 / millis;
}
