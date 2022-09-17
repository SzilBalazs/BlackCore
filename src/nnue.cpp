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
#include <cstring>
#include <immintrin.h>
#include "nnue.h"
#include "position.h"

#include "incbin/incbin.h"

namespace NNUE {

    INCBIN(Net, "corenet.bin");

    alignas(64) int16_t L_0_WEIGHTS[L_0_SIZE * L_1_SIZE];
    alignas(64) int16_t L_0_BIASES[L_1_SIZE];

    alignas(64) int16_t L_1_WEIGHTS[L_1_SIZE * 1];
    alignas(64) int16_t L_1_BIASES[1];

    void Accumulator::loadAccumulator(NNUE::Accumulator &accumulator) {

        for (int i = 0; i < chunkNum; i += 4) {
            const int offset1 = (i + 0) * regWidth;
            const int offset2 = (i + 1) * regWidth;
            const int offset3 = (i + 2) * regWidth;
            const int offset4 = (i + 3) * regWidth;

            __m256i ac1 = _mm256_loadu_si256((__m256i *) &accumulator.hiddenLayer[offset1]);
            __m256i ac2 = _mm256_loadu_si256((__m256i *) &accumulator.hiddenLayer[offset2]);
            __m256i ac3 = _mm256_loadu_si256((__m256i *) &accumulator.hiddenLayer[offset3]);
            __m256i ac4 = _mm256_loadu_si256((__m256i *) &accumulator.hiddenLayer[offset4]);

            _mm256_storeu_si256((__m256i *) &hiddenLayer[offset1], ac1);
            _mm256_storeu_si256((__m256i *) &hiddenLayer[offset2], ac2);
            _mm256_storeu_si256((__m256i *) &hiddenLayer[offset3], ac3);
            _mm256_storeu_si256((__m256i *) &hiddenLayer[offset4], ac4);
        }

        // std::memcpy(hiddenLayer, accumulator.hiddenLayer, sizeof(int16_t) * L_1_SIZE);
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

    void Accumulator::addFeature(int index) {

        for (int i = 0; i < chunkNum; i += 4) {
            const int offset1 = (i + 0) * regWidth;
            const int offset2 = (i + 1) * regWidth;
            const int offset3 = (i + 2) * regWidth;
            const int offset4 = (i + 3) * regWidth;

            __m256i ac1 = _mm256_loadu_si256((__m256i *) &hiddenLayer[offset1]);
            __m256i ac2 = _mm256_loadu_si256((__m256i *) &hiddenLayer[offset2]);
            __m256i ac3 = _mm256_loadu_si256((__m256i *) &hiddenLayer[offset3]);
            __m256i ac4 = _mm256_loadu_si256((__m256i *) &hiddenLayer[offset4]);

            __m256i we1 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset1]);
            __m256i we2 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset2]);
            __m256i we3 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset3]);
            __m256i we4 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset4]);

            __m256i sum1 = _mm256_add_epi16(ac1, we1);
            __m256i sum2 = _mm256_add_epi16(ac2, we2);
            __m256i sum3 = _mm256_add_epi16(ac3, we3);
            __m256i sum4 = _mm256_add_epi16(ac4, we4);

            _mm256_storeu_si256((__m256i *) &hiddenLayer[offset1], sum1);
            _mm256_storeu_si256((__m256i *) &hiddenLayer[offset2], sum2);
            _mm256_storeu_si256((__m256i *) &hiddenLayer[offset3], sum3);
            _mm256_storeu_si256((__m256i *) &hiddenLayer[offset4], sum4);
        }

        /*for (int i = 0; i < L_1_SIZE; i++) {
            hiddenLayer[i] += L_0_WEIGHTS[index * L_1_SIZE + i];
        }*/
    }

    void Accumulator::removeFeature(int index) {

        for (int i = 0; i < chunkNum; i += 4) {

            const int offset1 = (i + 0) * regWidth;
            const int offset2 = (i + 1) * regWidth;
            const int offset3 = (i + 2) * regWidth;
            const int offset4 = (i + 3) * regWidth;

            __m256i ac1 = _mm256_loadu_si256((__m256i *) &hiddenLayer[offset1]);
            __m256i ac2 = _mm256_loadu_si256((__m256i *) &hiddenLayer[offset2]);
            __m256i ac3 = _mm256_loadu_si256((__m256i *) &hiddenLayer[offset3]);
            __m256i ac4 = _mm256_loadu_si256((__m256i *) &hiddenLayer[offset4]);

            __m256i we1 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset1]);
            __m256i we2 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset2]);
            __m256i we3 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset3]);
            __m256i we4 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset4]);

            __m256i sum1 = _mm256_sub_epi16(ac1, we1);
            __m256i sum2 = _mm256_sub_epi16(ac2, we2);
            __m256i sum3 = _mm256_sub_epi16(ac3, we3);
            __m256i sum4 = _mm256_sub_epi16(ac4, we4);

            _mm256_storeu_si256((__m256i *) &hiddenLayer[offset1], sum1);
            _mm256_storeu_si256((__m256i *) &hiddenLayer[offset2], sum2);
            _mm256_storeu_si256((__m256i *) &hiddenLayer[offset3], sum3);
            _mm256_storeu_si256((__m256i *) &hiddenLayer[offset4], sum4);
        }

        /*for (int i = 0; i < L_1_SIZE; i++) {
            hiddenLayer[i] -= L_0_WEIGHTS[index * L_1_SIZE + i];
        }*/
    }

    Score Accumulator::forward() {

        int32_t output = L_1_BIASES[0];

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

            int ptr = 0;
            std::memcpy(L_0_WEIGHTS, gNetData + ptr, sizeof(int16_t) * L_0_SIZE * L_1_SIZE);
            ptr += sizeof(int16_t) * L_0_SIZE * L_1_SIZE;
            std::memcpy(L_0_BIASES, gNetData + ptr, sizeof(int16_t) * L_1_SIZE);
            ptr += sizeof(int16_t) * L_1_SIZE;
            std::memcpy(L_1_WEIGHTS, gNetData + ptr, sizeof(int16_t) * L_1_SIZE * 1);
            ptr += sizeof(int16_t) * L_1_SIZE * 1;
            std::memcpy(L_1_BIASES, gNetData + ptr, sizeof(int16_t) * 1);
            ptr += sizeof(int16_t) * 1;

        }

    }
}