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

std::chrono::steady_clock::time_point searchBegin;

void startSearch() {
    nodeCount = 0;
    searchBegin = std::chrono::steady_clock::now();
}

U64 getSearchTime() {
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - searchBegin).count();
}

U64 getNps() {
    U64 millis = getSearchTime();
    return millis == 0 ? 0 : nodeCount * 1000 / millis;
}
