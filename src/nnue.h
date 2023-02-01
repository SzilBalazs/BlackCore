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

    extern std::string EVALFILE;

    constexpr int KING_BUCKET_COUNT = 4;
    constexpr int L_0_SIZE = KING_BUCKET_COUNT * 768;
    constexpr int L_1_SIZE = 256;

    // clang-format off
    constexpr int KING_BUCKET[64]{
        0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 1, 1, 1, 1,
        2, 2, 2, 2, 3, 3, 3, 3,
        2, 2, 2, 2, 3, 3, 3, 3,
        2, 2, 2, 2, 3, 3, 3, 3,
        2, 2, 2, 2, 3, 3, 3, 3,
    };
    // clang-format on

    constexpr int regWidth = 256 / 16;
    constexpr int chunkNum = 256 / regWidth;

    // Stores the hidden layer of the NNUE.
    struct Accumulator {
        alignas(32) int16_t hiddenLayer[2][L_1_SIZE];

        constexpr Accumulator() {}

        void loadAccumulator(Accumulator &accumulator);

        void refresh(const Position &pos);

        void addFeature(Color pieceColor, PieceType pieceType, Square sq, Square wKing, Square bKing);

        void removeFeature(Color pieceColor, PieceType pieceType, Square sq, Square wKing, Square bKing);

        Score forward(Color stm);
    };

    // Returns the L_0 index of a feature.
    constexpr int getInputIndex(Color perspective, Color pieceColor, PieceType pieceType, Square sq, Square kingSquare) {
        return (perspective == WHITE ? pieceColor : 1 - pieceColor) * 384 + pieceType * 64 +
               (perspective == WHITE ? sq : sq ^ 56) + KING_BUCKET[kingSquare] * 768;
    }

    // Activation function used in BlackCore's NNUE.
    constexpr int16_t ReLU(int16_t in) {
        return std::max((int16_t) 0, in);
    }

    void init();
} // namespace NNUE

#endif //BLACKCORE_NNUE_H
