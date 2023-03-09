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

#pragma once

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

#define TUNING_PARAMETERS               \
    DEPTH_PARAM(ASPIRATION_DEPTH, 100)  \
    SCORE_PARAM(ASPIRATION_DELTA, 30)   \
    SCORE_PARAM(ASPIRATION_BOUND, 3000) \
    DEPTH_PARAM(RFP_DEPTH, 7)           \
    SCORE_PARAM(RFP_MULTI, 70)          \
    DEPTH_PARAM(NMP_DEPTH, 3)           \
    SCORE_PARAM(NMP_BASE, 3)            \
    SCORE_PARAM(NMP_DEPTH_MULTI, 5)     \
    DEPTH_PARAM(LMR_DEPTH, 3)           \
    SCORE_PARAM(LMR_BASE_INDEX, 3)      \
    SCORE_PARAM(LMR_PV_INDEX, 2)

TUNING_PARAMETERS

#undef DEPTH_PARAM
#undef SCORE_PARAM
