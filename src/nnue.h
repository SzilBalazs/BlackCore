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

#ifndef BLACKCORE_NNUE_H
#define BLACKCORE_NNUE_H

#include "bitboard.h"

class Position;

namespace NNUE {

    /* +-------------------+
     * | NNUE Architecture |
     * | 768->32->32->1    |
     * +-------------------+
     *
     * L_0_IN = features count
     *
     * L_1_WEIGHTS = in features -> L_1 (768 -> 32)
     * L_1_BIASES = L_1 biases
     *
     * L_2_WEIGHTS = L_1 -> L_2 (32 -> 32)
     * L_2_BIASES = L_2 biases
     *
     */


    constexpr unsigned int L_0_IN = 2 * 6 * 64;
    constexpr unsigned int L_1_SIZE = 32;
    constexpr unsigned int L_2_SIZE = 32;

    struct Accumulator {
        int16_t hiddenLayer[L_1_SIZE];

        constexpr Accumulator() = default;

        void loadAccumulator(Accumulator &accumulator);

        void refresh(const Position &pos);

        void addFeature(unsigned int index);

        void removeFeature(unsigned int index);

        Score forward();
    };

    constexpr unsigned int getAccumulatorIndex(Color color, PieceType type, Square square) {
        return color * 384 + type * 64 + square;
    }

    constexpr int16_t ReLu(int16_t in) {
        return std::max((int16_t) 0, in);
    }

    void init();
}

#endif //BLACKCORE_NNUE_H
