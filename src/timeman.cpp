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

using namespace std::chrono;

void TimeManager::init(SearchInfo searchInfo, Color stm) {

    startPoint = now();

    int64_t timeLeft, movesLeft, increment;
    movesLeft = searchInfo.movestogo;

    if (movesLeft == 0) {
        movesLeft = 25;
    }

    if (stm == WHITE) {
        timeLeft = searchInfo.wtime;
        increment = searchInfo.winc;
    } else {
        timeLeft = searchInfo.btime;
        increment = searchInfo.binc;
    }

    if (searchInfo.movetime != -1) {
        optimum = searchInfo.movetime;
        maximum = searchInfo.movetime;
    } else if (timeLeft == -1) {
        optimum = INT32_MAX;
        maximum = INT32_MAX;
    } else {
        optimum = timeLeft / movesLeft + increment;
        maximum = timeLeft / movesLeft + increment;
    }
}

int64_t TimeManager::calcNps(int64_t nodes) const {
    int64_t elapsedTime = now() - startPoint;
    return elapsedTime == 0 ? 0 : nodes * 1000 / elapsedTime;
}

bool TimeManager::resourcesLeft() {
    return now() < startPoint + optimum;
}

int64_t TimeManager::now() const {
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}