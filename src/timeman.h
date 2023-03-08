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

#include "uci.h"
#include <atomic>

class TimeManager {

public:
    TimeManager() = default;

    void init(SearchInfo searchInfo, Color stm);

    bool resourcesLeft();

    inline void setOverhead(int64_t x) {
        overhead = x;
    }

    bool scaleOptimum(double scale);

    int64_t elapsedTime() const;

    int64_t calcNps(int64_t nodes) const;

private:
    int64_t optimum, maximum, startPoint, overhead = 30;

    int64_t now() const;
};
