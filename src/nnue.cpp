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

namespace NNUE {

    int16_t L_1_WEIGHTS[L_0_IN * L_1_SIZE];
    int16_t L_1_BIASES[L_1_SIZE];

    int16_t L_2_WEIGHTS[L_1_SIZE * L_2_SIZE];
    int16_t L_2_BIASES[L_2_SIZE];

    int16_t OUT_WEIGHTS[L_2_SIZE];

    Accumulator::Accumulator(const Position &pos) {
        refresh(pos);
    }

    Accumulator::Accumulator(NNUE::Accumulator &accumulator) {
        std::memcpy(hiddenLayer, accumulator.hiddenLayer, sizeof(int16_t) * L_1_SIZE);
    }

    void Accumulator::refresh(const Position &pos) {
        std::memcpy(hiddenLayer, L_1_BIASES, sizeof(int16_t) * L_1_SIZE);

        for (Color color : {WHITE, BLACK}) {
            for (PieceType type : {KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN}) {
                Bitboard bb = pos.pieces(color, type);
                while (bb) {
                    addFeature(color * 384 + type * 64 + bb.popLsb());
                }
            }
        }
    }

    void Accumulator::addFeature(unsigned int index) {
        for (unsigned int i = 0; i < L_1_SIZE; i++) {
            hiddenLayer[i] += L_1_WEIGHTS[index * L_1_SIZE + i];
        }
    }

    void Accumulator::removeFeature(unsigned int index) {
        for (unsigned int i = 0; i < L_1_SIZE; i++) {
            hiddenLayer[i] -= L_1_WEIGHTS[index * L_1_SIZE + i];
        }
    }


    void init() {

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