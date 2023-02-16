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

#ifndef BLACKCORE_TIMEMAN_H
#define BLACKCORE_TIMEMAN_H

#include "constants.h"
#include <atomic>

extern unsigned int MOVE_OVERHEAD;
extern std::atomic<bool> stopped;

void initTimeManager(long long time, long long inc, long long movesToGo, long long moveTime, long long nodes);

bool shouldEnd(U64 nodes, U64 totalNodes);

bool manageTime(double factor);

bool isInfiniteSearch();

long long getSearchTime();

U64 getNps(U64 nodes);

#endif //BLACKCORE_TIMEMAN_H
