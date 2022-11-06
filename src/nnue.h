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

    /*
     * +-----------------------+
     * |   NNUE Architecture   |
     * |    2x(768->256)->1    |
     * |    Activation: ReLU   |
     * +-----------------------+
     *
     * L_0_SIZE = features count
     *
     * L_0_WEIGHTS = in features -> L_1 (768 -> 256)
     * L_0_BIASES = L_1 biases
     *
     * L_1_WEIGHTS = 2xL_1 -> L_2 (2x256 -> 1)
     * L_1_BIASES = L_2 biases
     *
     */


    constexpr int L_0_SIZE = 768;
    constexpr int L_1_SIZE = 256;

    constexpr int regWidth = 256 / 16;
    constexpr int chunkNum = 256 / regWidth;

    struct Accumulator {
        alignas(32) int16_t hiddenLayer[2][L_1_SIZE];

        constexpr Accumulator() {}

        void loadAccumulator(Accumulator &accumulator);

        void refresh(const Position &pos);

        void addFeature(Color pieceColor, PieceType pieceType, Square sq);

        void removeFeature(Color pieceColor, PieceType pieceType, Square sq);

        Score forward(Color stm);
    };

    constexpr int getAccumulatorIndex(Color perspective, Color pieceColor, PieceType pieceType, Square square) {
        return (perspective == WHITE ? pieceColor : 1 - pieceColor) * 384 + pieceType * 64 +
               (perspective == WHITE ? square : square ^ 56);
    }

    constexpr int16_t ReLU(int16_t in) {
        return std::max((int16_t) 0, in);
    }

    void init();
}

#endif //BLACKCORE_NNUE_H
