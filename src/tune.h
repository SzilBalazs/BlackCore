// BlackCore is a chess engine
// Copyright (c) 2023 SzilBalazs
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

#ifndef BLACKCORE_TUNE_H
#define BLACKCORE_TUNE_H

#include <string>
#include <utility>
#include <vector>

#include "constants.h"

#ifndef TUNE

#define SCORE_PARAM(NAME, VALUE) constexpr Score NAME = VALUE;
#define DEPTH_PARAM(NAME, VALUE) constexpr Depth NAME = VALUE;

#else

struct TuneParam {
    std::string name;
    int defaultValue;
    int minValue;
    int maxValue;
    int *ptr;

    TuneParam(std::string a, int x, int y, int z, int *p) : name(std::move(a)), defaultValue(x), minValue(y), maxValue(z), ptr(p) {}
};

extern std::vector<TuneParam> params;

void tuneInit();

#define SCORE_PARAM(NAME, VALUE) extern Score NAME;
#define DEPTH_PARAM(NAME, VALUE) extern Score NAME; // Use the same type of score param

inline void addParam(const std::string &name, int defaultValue, int minValue, int maxValue, int *ptr) {
    params.emplace_back(name, defaultValue, minValue, maxValue, ptr);
}

#endif

#define TUNING_PARAMETERS                      \
    SCORE_PARAM(DELTA_MARGIN, 252)             \
                                               \
    SCORE_PARAM(RAZOR_MARGIN, 155)             \
                                               \
    DEPTH_PARAM(RFP_DEPTH, 8)                  \
    SCORE_PARAM(RFP_DEPTH_MULTIPLIER, 42)      \
    SCORE_PARAM(RFP_IMPROVING_MULTIPLIER, 66)  \
                                               \
    DEPTH_PARAM(NULL_MOVE_DEPTH, 2)            \
    DEPTH_PARAM(NULL_MOVE_BASE_R, 4)           \
    DEPTH_PARAM(NULL_MOVE_R_SCALE, 3)          \
                                               \
    DEPTH_PARAM(LMR_DEPTH, 3)                  \
    SCORE_PARAM(LMR_INDEX, 3)                  \
                                               \
    DEPTH_PARAM(LMP_DEPTH, 4)                  \
    SCORE_PARAM(LMP_MOVES, 5)                  \
                                               \
    DEPTH_PARAM(FUTILITY_DEPTH, 5)             \
    SCORE_PARAM(FUTILITY_MARGIN, 33)           \
    SCORE_PARAM(FUTILITY_MARGIN_DEPTH, 53)     \
    SCORE_PARAM(FUTILITY_MARGIN_IMPROVING, 71) \
                                               \
    DEPTH_PARAM(ASPIRATION_DEPTH, 5)           \
    SCORE_PARAM(ASPIRATION_DELTA, 23)          \
    SCORE_PARAM(ASPIRATION_BOUND, 2000)        \
                                               \
    DEPTH_PARAM(SINGULAR_DEPTH, 8)

constexpr double LMR_BASE = 0.1;
constexpr double LMR_SCALE = 1.6;

TUNING_PARAMETERS

#undef DEPTH_PARAM
#undef SCORE_PARAM

#endif //BLACKCORE_TUNE_H
