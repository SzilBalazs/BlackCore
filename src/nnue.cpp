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

    int16_t L_0_WEIGHTS[L_0_SIZE * L_1_SIZE];
    int16_t L_0_BIASES[L_1_SIZE];

    int16_t L_1_WEIGHTS[L_1_SIZE * 1];
    int16_t L_1_BIASES[1];


    void Accumulator::loadAccumulator(NNUE::Accumulator &accumulator) {
        std::memcpy(hiddenLayer, accumulator.hiddenLayer, sizeof(int16_t) * L_1_SIZE);
    }

    void Accumulator::refresh(const Position &pos) {
        std::memcpy(hiddenLayer, L_0_BIASES, sizeof(int16_t) * L_1_SIZE);

        for (Square sq = A1; sq < 64; sq += 1) {
            Piece p = pos.pieceAt(sq);
            if (!p.isNull()) {
                addFeature(getAccumulatorIndex(p.color, p.type, sq));
            }
        }
    }

    void Accumulator::addFeature(unsigned int index) {
        for (int i = 0; i < L_1_SIZE; i++) {
            hiddenLayer[i] += L_0_WEIGHTS[i * L_0_SIZE + index];
        }
    }

    void Accumulator::removeFeature(unsigned int index) {
        for (int i = 0; i < L_1_SIZE; i++) {
            hiddenLayer[i] -= L_0_WEIGHTS[i * L_0_SIZE + index];
        }
    }

    Score Accumulator::forward() {

        int32_t output = L_1_BIASES[0] * 64;

        for (int i = 0; i < L_1_SIZE; i++) {
            output += ReLU(hiddenLayer[i]) * L_1_WEIGHTS[i];
        }

        return output * 400 / (255 * 255);
    }

    void init() {

        std::memset(L_0_WEIGHTS, 0, sizeof(L_0_WEIGHTS));
        std::memset(L_0_BIASES, 0, sizeof(L_0_BIASES));
        std::memset(L_1_WEIGHTS, 0, sizeof(L_1_WEIGHTS));
        std::memset(L_1_BIASES, 0, sizeof(L_1_BIASES));

        FILE *file = fopen("corenet.bin", "rb");

        if (file != nullptr) {
            fread(L_0_WEIGHTS, sizeof(int16_t), L_0_SIZE * L_1_SIZE, file);
            fread(L_0_BIASES, sizeof(int16_t), L_1_SIZE, file);

            fread(L_1_WEIGHTS, sizeof(int16_t), L_1_SIZE * 1, file);
            fread(L_1_BIASES, sizeof(int16_t), 1, file);

        } else {
            std::cout << "No net was found! Please download the nnue.net with the executable" << std::endl;
            exit(1);
        }

    }
}