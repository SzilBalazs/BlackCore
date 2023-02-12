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

#include "tune.h"
#include <iostream>

#ifdef TUNE

std::vector<TuneParam> params;

#define SCORE_PARAM(NAME, VALUE) Score NAME = VALUE;
#define DEPTH_PARAM(NAME, VALUE) Score NAME = VALUE;

TUNING_PARAMETERS

#undef SCORE_PARAM
#undef DEPTH_PARAM

#define SCORE_PARAM(NAME, VALUE, MIN_VALUE, MAX_VALUE)   \
    addParam(#NAME, VALUE, MIN_VALUE, MAX_VALUE, &NAME); \
    NAME = VALUE;

#define DEPTH_PARAM(NAME, VALUE, MIN_VALUE, MAX_VALUE)   \
    addParam(#NAME, VALUE, MIN_VALUE, MAX_VALUE, &NAME); \
    NAME = VALUE;

void tuneInit() {

    SCORE_PARAM(DELTA_MARGIN, 252, 100, 500)

    SCORE_PARAM(RAZOR_MARGIN, 155, 100, 500)

    DEPTH_PARAM(RFP_DEPTH, 8, 5, 10)
    SCORE_PARAM(RFP_DEPTH_MULTIPLIER, 42, 30, 100)
    SCORE_PARAM(RFP_IMPROVING_MULTIPLIER, 66, 30, 100)

    DEPTH_PARAM(NULL_MOVE_DEPTH, 2, 2, 10)
    DEPTH_PARAM(NULL_MOVE_BASE_R, 4, 2, 10)
    DEPTH_PARAM(NULL_MOVE_R_SCALE, 3, 2, 10)

    DEPTH_PARAM(LMR_DEPTH, 3, 2, 10)
    SCORE_PARAM(LMR_INDEX, 3, 2, 10)

    DEPTH_PARAM(LMP_DEPTH, 4, 2, 10)
    SCORE_PARAM(LMP_MOVES, 5, 2, 10)

    DEPTH_PARAM(FUTILITY_DEPTH, 5, 2, 10)
    SCORE_PARAM(FUTILITY_MARGIN, 33, 10, 150)
    SCORE_PARAM(FUTILITY_MARGIN_DEPTH, 53, 10, 150)
    SCORE_PARAM(FUTILITY_MARGIN_IMPROVING, 71, 10, 150)

    DEPTH_PARAM(ASPIRATION_DEPTH, 9, 2, 15)
    SCORE_PARAM(ASPIRATION_DELTA, 28, 10, 50)
    SCORE_PARAM(ASPIRATION_BOUND, 3000, 2000, 4000)

    DEPTH_PARAM(SINGULAR_DEPTH, 8, 5, 15)

    for (const auto &param : params) {
        std::cout << "option name " << param.name << " type spin default " << param.defaultValue << " min " << param.minValue << " max " << param.maxValue << std::endl;
    }

    /*for (const auto &param : params) {
        std::cout << "\"" << param.name
                  << "\": {\n        \"value\": " << param.defaultValue
                  << ",\n        \"min_value\": " << param.minValue << ",\n"
                  << "        \"max_value\": " << param.maxValue << ",\n"
                  << "        \"step\": X\n"
                  << "    },\n    ";
    }
    std::cout << std::endl;*/
}

#endif