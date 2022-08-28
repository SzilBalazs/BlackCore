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

#ifdef TUNE

extern Score DELTA_MARGIN;

extern Score RAZOR_MARGIN;

extern Depth RFP_DEPTH;
extern Score RFP_DEPTH_MULTIPLIER;
extern Score RFP_IMPROVING_MULTIPLIER;

extern Depth NULL_MOVE_DEPTH;
extern Depth NULL_MOVE_BASE_R;
extern Depth NULL_MOVE_R_SCALE;

extern Depth LMR_DEPTH;
extern double LMR_BASE;
extern double LMR_SCALE;
extern int LMR_MIN_I;
extern int LMR_PVNODE_I;

extern Score SEE_PRUNING_MARGIN;

extern Depth IID_DEPTH;

#else
constexpr Score DELTA_MARGIN = 400;

constexpr Score RAZOR_MARGIN = 130;

constexpr Depth RFP_DEPTH = 5;
constexpr Score RFP_DEPTH_MULTIPLIER = 80;
constexpr Score RFP_IMPROVING_MULTIPLIER = 90;

constexpr Depth NULL_MOVE_DEPTH = 3;
constexpr Depth NULL_MOVE_BASE_R = 4;
constexpr Depth NULL_MOVE_R_SCALE = 5;

constexpr Depth LMR_DEPTH = 4;
constexpr double LMR_BASE = 1;
constexpr double LMR_SCALE = 1.75;
constexpr int LMR_MIN_I = 3;
constexpr int LMR_PVNODE_I = 3;

constexpr Score SEE_PRUNING_MARGIN = 150;

constexpr Depth IID_DEPTH = 5;
#endif

struct SearchState {
    Move move;
    Score eval = 0;
};

void initLmr();

Score see(const Position &pos, Move move);

void iterativeDeepening(Position pos, Depth depth, bool uci);

#endif //BLACKCORE_SEARCH_H
