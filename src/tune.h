#ifndef BLACKCORE_TUNE_H
#define BLACKCORE_TUNE_H

#include <iostream>
#include <map>
#include <vector>

#include "constants.h"

// #define TUNE

namespace Tune {
    struct TuneEntry {
        int value, minValue, maxValue;
        size_t index;
    };

    extern std::map<std::string, TuneEntry> tuneParams;
    extern std::vector<int> tuneValues;

    inline void ADD_TUNE_PARAM(const std::string &name, int value, int minValue, int maxValue) {
        tuneParams[name] = {value, minValue, maxValue, tuneValues.size()};
        tuneValues.emplace_back(value);
    }

    inline void initTune() {

        ADD_TUNE_PARAM("DELTA_MARGIN", 252, 150, 400);

        ADD_TUNE_PARAM("RAZOR_MARGIN", 155, 100, 200);

        ADD_TUNE_PARAM("RFP_DEPTH", 8, 5, 10);
        ADD_TUNE_PARAM("RFP_DEPTH_MULTIPLIER", 42, 30, 100);
        ADD_TUNE_PARAM("RFP_IMPROVING_MULTIPLIER", 66, 30, 100);

        ADD_TUNE_PARAM("NULL_MOVE_DEPTH", 2, 1, 6);
        ADD_TUNE_PARAM("NULL_MOVE_BASE_R", 4, 1, 6);
        ADD_TUNE_PARAM("NULL_MOVE_R_SCALE", 2, 1, 10);

        ADD_TUNE_PARAM("LMR_DEPTH", 3, 2, 10);
        ADD_TUNE_PARAM("LMR_BASE", 0, 0, 200);
        ADD_TUNE_PARAM("LMR_SCALE", 165, 100, 200);
        ADD_TUNE_PARAM("LMR_INDEX", 2, 1, 10);

        ADD_TUNE_PARAM("LMP_DEPTH", 4, 1, 10);
        ADD_TUNE_PARAM("LMP_MOVES", 5, 1, 10);

        ADD_TUNE_PARAM("FUTILITY_DEPTH", 3, 1, 10);
        ADD_TUNE_PARAM("FUTILITY_MARGIN", 30, 10, 100);
        ADD_TUNE_PARAM("FUTILITY_MARGIN_DEPTH", 60, 10, 100);
        ADD_TUNE_PARAM("FUTILITY_MARGIN_IMPROVING", 80, 10, 150);

        ADD_TUNE_PARAM("ASPIRATION_DEPTH", 9, 5, 15);
        ADD_TUNE_PARAM("ASPIRATION_DELTA", 28, 20, 40);
        ADD_TUNE_PARAM("ASPIRATION_BOUND", 3000, 2000, 4000);

        ADD_TUNE_PARAM("SINGULAR_DEPTH", 7, 5, 15);

#ifdef TUNE
        for (const auto &entry : tuneParams) {
            std::cout << "option name " << entry.first << " type spin default " << entry.second.value << " min " << entry.second.minValue << " max " << entry.second.maxValue << std::endl;
        }
#endif
    }

    inline void printTunePrepare() {
        for (const auto &entry : tuneParams) {
            std::cout << "#define " << entry.first << " Tune::tuneValues[" << entry.second.index << "]" << std::endl;
        }
        std::cout << std::endl
                  << "{\n    ";
        for (const auto &entry : tuneParams) {
            std::cout << "\"" << entry.first
                      << "\": {\n        \"value\": " << entry.second.value
                      << ",        \"min_value\": " << entry.second.minValue << ",\n"
                      << "        \"max_value\": " << entry.second.maxValue << ",\n"
                      << "        \"step\": X\n"
                      << "    },\n    ";
        }
    }

} // namespace Tune

#undef ADD_TUNE_PARAM

#ifdef TUNE

#define ASPIRATION_BOUND Tune::tuneValues[20]
#define ASPIRATION_DELTA Tune::tuneValues[19]
#define ASPIRATION_DEPTH Tune::tuneValues[18]
#define DELTA_MARGIN Tune::tuneValues[0]
#define FUTILITY_DEPTH Tune::tuneValues[14]
#define FUTILITY_MARGIN Tune::tuneValues[15]
#define FUTILITY_MARGIN_DEPTH Tune::tuneValues[16]
#define FUTILITY_MARGIN_IMPROVING Tune::tuneValues[17]
#define LMP_DEPTH Tune::tuneValues[12]
#define LMP_MOVES Tune::tuneValues[13]
#define LMR_BASE Tune::tuneValues[9]
#define LMR_DEPTH Tune::tuneValues[8]
#define LMR_INDEX Tune::tuneValues[11]
#define LMR_SCALE Tune::tuneValues[10]
#define NULL_MOVE_BASE_R Tune::tuneValues[6]
#define NULL_MOVE_DEPTH Tune::tuneValues[5]
#define NULL_MOVE_R_SCALE Tune::tuneValues[7]
#define RAZOR_MARGIN Tune::tuneValues[1]
#define RFP_DEPTH Tune::tuneValues[2]
#define RFP_DEPTH_MULTIPLIER Tune::tuneValues[3]
#define RFP_IMPROVING_MULTIPLIER Tune::tuneValues[4]
#define SINGULAR_DEPTH Tune::tuneValues[21]

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
constexpr int LMR_BASE = 0;
constexpr int LMR_SCALE = 165;
constexpr int LMR_INDEX = 2;

constexpr Depth LMP_DEPTH = 4;
constexpr int LMP_MOVES = 5;

constexpr Depth FUTILITY_DEPTH = 3;
constexpr Score FUTILITY_MARGIN = 30;
constexpr Score FUTILITY_MARGIN_DEPTH = 60;
constexpr Score FUTILITY_MARGIN_IMPROVING = 80;

constexpr Depth ASPIRATION_DEPTH = 9;
constexpr Score ASPIRATION_DELTA = 28;
constexpr Score ASPIRATION_BOUND = 3000;

constexpr Depth SINGULAR_DEPTH = 7;

#endif

#endif //BLACKCORE_TUNE_H
