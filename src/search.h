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

#ifndef BLACKCORE_SEARCH_H
#define BLACKCORE_SEARCH_H

#include "movegen.h"

constexpr Score DELTA_MARGIN = 400;

constexpr Score RAZOR_MARGIN = 130;

constexpr Depth RFP_DEPTH = 6;
constexpr Score RFP_DEPTH_MULTIPLIER = 160;

constexpr Depth NULL_MOVE_DEPTH = 3;
constexpr Depth NULL_MOVE_REDUCTION = 5;

struct SearchState {
    bool doNullMove = true;
    Score eval = 0;
};

void iterativeDeepening(Position pos, Depth depth, bool uci);

#endif //BLACKCORE_SEARCH_H
