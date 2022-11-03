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
#include <cstring>
#include <immintrin.h>
#include "nnue.h"
#include "position.h"

#include "incbin/incbin.h"

namespace NNUE {

    INCBIN(Net, "corenet.bin");

    alignas(64) int16_t L_0_WEIGHTS[L_0_SIZE * L_1_SIZE];
    alignas(64) int16_t L_0_BIASES[L_1_SIZE];

    alignas(64) int16_t L_1_WEIGHTS[L_1_SIZE * 2];
    alignas(64) int16_t L_1_BIASES[1];

    void Accumulator::loadAccumulator(NNUE::Accumulator &accumulator) {
        for (Color perspective : {WHITE, BLACK}) {
#ifdef AVX2
            for (int i = 0; i < chunkNum; i += 4) {
                const int offset1 = (i + 0) * regWidth;
                const int offset2 = (i + 1) * regWidth;
                const int offset3 = (i + 2) * regWidth;
                const int offset4 = (i + 3) * regWidth;

                __m256i ac1 = _mm256_loadu_si256((__m256i *) &accumulator.hiddenLayer[perspective][offset1]);
                __m256i ac2 = _mm256_loadu_si256((__m256i *) &accumulator.hiddenLayer[perspective][offset2]);
                __m256i ac3 = _mm256_loadu_si256((__m256i *) &accumulator.hiddenLayer[perspective][offset3]);
                __m256i ac4 = _mm256_loadu_si256((__m256i *) &accumulator.hiddenLayer[perspective][offset4]);

                _mm256_storeu_si256((__m256i *) &hiddenLayer[perspective][offset1], ac1);
                _mm256_storeu_si256((__m256i *) &hiddenLayer[perspective][offset2], ac2);
                _mm256_storeu_si256((__m256i *) &hiddenLayer[perspective][offset3], ac3);
                _mm256_storeu_si256((__m256i *) &hiddenLayer[perspective][offset4], ac4);
            }
#else
            std::memcpy(hiddenLayer[perspective], accumulator.hiddenLayer[perspective], sizeof(int16_t) * L_1_SIZE * 1);
#endif
        }

    }

    void Accumulator::refresh(const Position &pos) {
        for (Color perspective : {WHITE, BLACK}) {
            std::memcpy(hiddenLayer[perspective], L_0_BIASES, sizeof(int16_t) * L_1_SIZE);
        }

        for (Square sq = A1; sq < 64; sq += 1) {
            Piece p = pos.pieceAt(sq);
            if (!p.isNull()) {
                addFeature(p.color, p.type, sq);
            }
        }
    }

    void Accumulator::addFeature(Color pieceColor, PieceType pieceType, Square sq) {
        for (Color perspective : {WHITE, BLACK}) {
            unsigned int index = getAccumulatorIndex(perspective, pieceColor, pieceType, sq);
#ifdef AVX2
            for (int i = 0; i < chunkNum; i += 4) {
                const int offset1 = (i + 0) * regWidth;
                const int offset2 = (i + 1) * regWidth;
                const int offset3 = (i + 2) * regWidth;
                const int offset4 = (i + 3) * regWidth;

                __m256i ac1 = _mm256_loadu_si256((__m256i *) &hiddenLayer[perspective][offset1]);
                __m256i ac2 = _mm256_loadu_si256((__m256i *) &hiddenLayer[perspective][offset2]);
                __m256i ac3 = _mm256_loadu_si256((__m256i *) &hiddenLayer[perspective][offset3]);
                __m256i ac4 = _mm256_loadu_si256((__m256i *) &hiddenLayer[perspective][offset4]);

                __m256i we1 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset1]);
                __m256i we2 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset2]);
                __m256i we3 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset3]);
                __m256i we4 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset4]);

                __m256i sum1 = _mm256_add_epi16(ac1, we1);
                __m256i sum2 = _mm256_add_epi16(ac2, we2);
                __m256i sum3 = _mm256_add_epi16(ac3, we3);
                __m256i sum4 = _mm256_add_epi16(ac4, we4);

                _mm256_storeu_si256((__m256i *) &hiddenLayer[perspective][offset1], sum1);
                _mm256_storeu_si256((__m256i *) &hiddenLayer[perspective][offset2], sum2);
                _mm256_storeu_si256((__m256i *) &hiddenLayer[perspective][offset3], sum3);
                _mm256_storeu_si256((__m256i *) &hiddenLayer[perspective][offset4], sum4);
            }
#else
            for (int i = 0; i < L_1_SIZE; i++) {
                hiddenLayer[perspective][i] += L_0_WEIGHTS[index * L_1_SIZE + i];
            }
#endif
        }
    }

    void Accumulator::removeFeature(Color pieceColor, PieceType pieceType, Square sq) {
        for (Color perspective : {WHITE, BLACK}) {
            unsigned int index = getAccumulatorIndex(perspective, pieceColor, pieceType, sq);
#ifdef AVX2
            for (int i = 0; i < chunkNum; i += 4) {

                const int offset1 = (i + 0) * regWidth;
                const int offset2 = (i + 1) * regWidth;
                const int offset3 = (i + 2) * regWidth;
                const int offset4 = (i + 3) * regWidth;

                __m256i ac1 = _mm256_loadu_si256((__m256i *) &hiddenLayer[perspective][offset1]);
                __m256i ac2 = _mm256_loadu_si256((__m256i *) &hiddenLayer[perspective][offset2]);
                __m256i ac3 = _mm256_loadu_si256((__m256i *) &hiddenLayer[perspective][offset3]);
                __m256i ac4 = _mm256_loadu_si256((__m256i *) &hiddenLayer[perspective][offset4]);

                __m256i we1 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset1]);
                __m256i we2 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset2]);
                __m256i we3 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset3]);
                __m256i we4 = _mm256_loadu_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset4]);

                __m256i sum1 = _mm256_sub_epi16(ac1, we1);
                __m256i sum2 = _mm256_sub_epi16(ac2, we2);
                __m256i sum3 = _mm256_sub_epi16(ac3, we3);
                __m256i sum4 = _mm256_sub_epi16(ac4, we4);

                _mm256_storeu_si256((__m256i *) &hiddenLayer[perspective][offset1], sum1);
                _mm256_storeu_si256((__m256i *) &hiddenLayer[perspective][offset2], sum2);
                _mm256_storeu_si256((__m256i *) &hiddenLayer[perspective][offset3], sum3);
                _mm256_storeu_si256((__m256i *) &hiddenLayer[perspective][offset4], sum4);
            }
#else
            for (int i = 0; i < L_1_SIZE; i++) {
                hiddenLayer[perspective][i] -= L_0_WEIGHTS[index * L_1_SIZE + i];
            }
#endif
        }
    }

    Score Accumulator::forward(Color stm) {

        int32_t output = L_1_BIASES[0];

        if (stm == WHITE) {
            for (int i = 0; i < L_1_SIZE; i++) {
                output += ReLU(hiddenLayer[WHITE][i]) * L_1_WEIGHTS[i];
            }

            for (int i = 0; i < L_1_SIZE; i++) {
                output += ReLU(hiddenLayer[BLACK][i]) * L_1_WEIGHTS[L_1_SIZE + i];
            }
        } else {
            for (int i = 0; i < L_1_SIZE; i++) {
                output += ReLU(hiddenLayer[BLACK][i]) * L_1_WEIGHTS[i];
            }

            for (int i = 0; i < L_1_SIZE; i++) {
                output += ReLU(hiddenLayer[WHITE][i]) * L_1_WEIGHTS[L_1_SIZE + i];
            }
        }

        return output * 400 / (255 * 255);
    }

    void init() {

        int ptr = 0;
        std::memcpy(L_0_WEIGHTS, gNetData + ptr, sizeof(int16_t) * L_0_SIZE * L_1_SIZE);
        ptr += sizeof(int16_t) * L_0_SIZE * L_1_SIZE;
        std::memcpy(L_0_BIASES, gNetData + ptr, sizeof(int16_t) * L_1_SIZE);
        ptr += sizeof(int16_t) * L_1_SIZE;
        std::memcpy(L_1_WEIGHTS, gNetData + ptr, sizeof(int16_t) * L_1_SIZE * 2);
        ptr += sizeof(int16_t) * L_1_SIZE * 2;
        std::memcpy(L_1_BIASES, gNetData + ptr, sizeof(int16_t) * 1);

        // Currently loading net from a file is not supported
        // Legacy code:

        /*FILE *file = fopen(filename.c_str(), "rb");

        if (file != nullptr) {

            fread(L_0_WEIGHTS, sizeof(int16_t), L_0_SIZE * L_1_SIZE, file);
            fread(L_0_BIASES, sizeof(int16_t), L_1_SIZE, file);
            fread(L_1_WEIGHTS, sizeof(int16_t), L_1_SIZE * 2, file);
            fread(L_1_BIASES, sizeof(int16_t), 1, file);

        }*/
    }
}
