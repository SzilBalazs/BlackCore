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

#include <atomic>
#include "movegen.h"
#include "uci.h"

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
extern int LMR_INDEX;

extern Depth LMP_DEPTH;
extern int LMP_MOVES;

extern Depth ASPIRATION_DEPTH;
extern Score ASPIRATION_DELTA;
extern Score ASPIRATION_BOUND;

extern Score SEE_MARGIN;

#else

constexpr Score DELTA_MARGIN = 252;

constexpr Score RAZOR_MARGIN = 155;

constexpr Depth RFP_DEPTH = 8;
constexpr Score RFP_DEPTH_MULTIPLIER = 42;
constexpr Score RFP_IMPROVING_MULTIPLIER = 66;

constexpr Depth NULL_MOVE_DEPTH = 2;
constexpr Depth NULL_MOVE_BASE_R = 4;
constexpr Depth NULL_MOVE_R_SCALE = 2;

constexpr Depth LMR_DEPTH = 3;
constexpr double LMR_BASE = 1;
constexpr double LMR_SCALE = 1.65;
constexpr int LMR_INDEX = 2;

constexpr Depth LMP_DEPTH = 4;
constexpr int LMP_MOVES = 5;

constexpr Depth ASPIRATION_DEPTH = 9;
constexpr Score ASPIRATION_DELTA = 28;
constexpr Score ASPIRATION_BOUND = 3000;

constexpr Score SEE_MARGIN = 2;

#endif

struct SearchStack {
    Move move;
    Score eval = 0;
};

void initLmr();

inline void initSearch() {
    initBitboard();
    initLmr();
    NNUE::init();
}

Score see(const Position &pos, Move move);

void joinThread(bool waitToFinish);

void startSearch(SearchInfo &searchInfo, Position &pos, int threadCount);

#endif //BLACKCORE_SEARCH_H
