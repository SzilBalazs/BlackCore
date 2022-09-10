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

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <cstring>
#include "nnue.h"
#include "position.h"

namespace NNUE {

    int16_t L_1_WEIGHTS[L_0_IN * L_1_SIZE];
    int16_t L_1_BIASES[L_1_SIZE];

    int16_t L_2_WEIGHTS[L_1_SIZE * L_2_SIZE];
    int16_t L_2_BIASES[L_2_SIZE];

    int16_t OUT_WEIGHTS[L_2_SIZE];

    void Accumulator::loadAccumulator(NNUE::Accumulator &accumulator) {
        std::memcpy(hiddenLayer, accumulator.hiddenLayer, sizeof(int16_t) * L_1_SIZE);
    }

    void Accumulator::refresh(const Position &pos) {
        std::memcpy(hiddenLayer, L_1_BIASES, sizeof(int16_t) * L_1_SIZE);

        for (Square sq = A1; sq < 64; sq += 1) {
            Piece p = pos.pieceAt(sq);
            if (!p.isNull()) {
                addFeature(getAccumulatorIndex(p.color, p.type, sq));
            }
        }
    }

    void Accumulator::addFeature(unsigned int index) {
        for (int i = 0; i < L_1_SIZE; i++) {
            hiddenLayer[i] += L_1_WEIGHTS[index * L_1_SIZE + i];
        }
    }

    void Accumulator::removeFeature(unsigned int index) {
        for (int i = 0; i < L_1_SIZE; i++) {
            hiddenLayer[i] -= L_1_WEIGHTS[index * L_1_SIZE + i];
        }
    }

    Score Accumulator::forward() {

        int16_t layer2[L_2_SIZE];

        std::memcpy(layer2, L_2_BIASES, sizeof(int16_t) * L_2_SIZE);

        for (int inIndex = 0; inIndex < L_1_SIZE; inIndex++) {
            for (int outIndex = 0; outIndex < L_2_SIZE; outIndex++) {
                layer2[outIndex] += ReLu(hiddenLayer[inIndex]) * L_2_WEIGHTS[inIndex * L_2_SIZE + outIndex];
            }
        }

        Score output = 0;

        for (int inIndex = 0; inIndex < L_2_SIZE; inIndex++) {
            output += ReLu(layer2[inIndex]) * OUT_WEIGHTS[inIndex];
        }

        return output;
    }

    void init() {

        std::memset(L_1_WEIGHTS, 0, sizeof(L_1_WEIGHTS));
        std::memset(L_1_BIASES, 0, sizeof(L_1_BIASES));
        std::memset(L_2_WEIGHTS, 0, sizeof(L_2_WEIGHTS));
        std::memset(L_2_BIASES, 0, sizeof(L_2_BIASES));
        std::memset(OUT_WEIGHTS, 0, sizeof(OUT_WEIGHTS));

        return;
        FILE *file = fopen("nnue.net", "rb");

        if (file != nullptr) {
            fread(L_1_WEIGHTS, sizeof(int16_t), L_0_IN * L_1_SIZE, file);
            fread(L_1_BIASES, sizeof(int16_t), L_1_SIZE, file);

            fread(L_2_WEIGHTS, sizeof(int16_t), L_1_SIZE * L_2_SIZE, file);
            fread(L_2_BIASES, sizeof(int16_t), L_2_SIZE, file);

            fread(OUT_WEIGHTS, sizeof(int16_t), L_2_SIZE, file);
        } else {
            std::cout << "No net was found! Please download the nnue.net with the executable" << std::endl;
            exit(1);
        }

    }
}