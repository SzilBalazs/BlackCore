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

#include "nnue.h"
#include "position.h"
#include <cstdint>
#include <cstring>
#include <immintrin.h>

#include "incbin/incbin.h"

namespace NNUE {

    INCBIN(Net, "corenet.bin");

    alignas(64) int16_t L_0_WEIGHTS[L_0_SIZE * L_1_SIZE];
    alignas(64) int16_t L_0_BIASES[L_1_SIZE];

    alignas(64) int16_t L_1_WEIGHTS[L_1_SIZE * 2];
    alignas(64) int16_t L_1_BIASES[1];

    // Copies accumulator.
    void Accumulator::loadAccumulator(NNUE::Accumulator &accumulator) {
        for (Color perspective : {WHITE, BLACK}) {
#ifdef AVX2
            for (int i = 0; i < chunkNum; i += 4) {
                const int offset1 = (i + 0) * regWidth;
                const int offset2 = (i + 1) * regWidth;
                const int offset3 = (i + 2) * regWidth;
                const int offset4 = (i + 3) * regWidth;

                __m256i ac1 = _mm256_load_si256((__m256i *) &accumulator.hiddenLayer[perspective][offset1]);
                __m256i ac2 = _mm256_load_si256((__m256i *) &accumulator.hiddenLayer[perspective][offset2]);
                __m256i ac3 = _mm256_load_si256((__m256i *) &accumulator.hiddenLayer[perspective][offset3]);
                __m256i ac4 = _mm256_load_si256((__m256i *) &accumulator.hiddenLayer[perspective][offset4]);

                _mm256_store_si256((__m256i *) &hiddenLayer[perspective][offset1], ac1);
                _mm256_store_si256((__m256i *) &hiddenLayer[perspective][offset2], ac2);
                _mm256_store_si256((__m256i *) &hiddenLayer[perspective][offset3], ac3);
                _mm256_store_si256((__m256i *) &hiddenLayer[perspective][offset4], ac4);
            }
#else
            std::memcpy(hiddenLayer[perspective], accumulator.hiddenLayer[perspective], sizeof(int16_t) * L_1_SIZE * 1);
#endif
        }
    }

    // Refreshes the accumulator.
    void Accumulator::refresh(const Position &pos) {

#ifdef AVX2
        __m256i registers[2][chunkNum];
        for (Color perspective : {WHITE, BLACK}) {
            for (int i = 0; i < chunkNum; i++) {
                const int offset = i * regWidth;
                registers[perspective][i] = _mm256_load_si256((__m256i *) &L_0_BIASES[offset]);
            }
        }
#else
        for (Color perspective : {WHITE, BLACK}) {
            std::memcpy(hiddenLayer[perspective], L_0_BIASES, sizeof(int16_t) * L_1_SIZE);
        }
#endif

        Bitboard occ = pos.occupied();

        Square wKing = pos.pieces<WHITE, KING>().lsb();
        Square bKing = pos.pieces<BLACK, KING>().lsb();

        while (occ) {
            Square sq = occ.popLsb();
            Piece p = pos.pieceAt(sq);
#ifdef AVX2
            for (Color perspective : {WHITE, BLACK}) {
                unsigned int index = getInputIndex(perspective, p.color, p.type, sq, ((perspective == WHITE) ? wKing : flipSquare(bKing)));
                for (int i = 0; i < chunkNum; i++) {
                    const int offset = i * regWidth;
                    __m256i we = _mm256_load_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset]);
                    registers[perspective][i] = _mm256_add_epi16(registers[perspective][i], we);
                }
            }

#else
            addFeature(p.color, p.type, sq, wKing, bKing);
#endif
        }

#ifdef AVX2
        for (Color perspective : {WHITE, BLACK}) {
            for (int i = 0; i < chunkNum; i++) {
                const int offset = i * regWidth;
                _mm256_store_si256((__m256i *) &hiddenLayer[perspective][offset], registers[perspective][i]);
            }
        }
#endif
    }

    // Adds a feature and forward propagates it to L_1.
    void Accumulator::addFeature(Color pieceColor, PieceType pieceType, Square sq, Square wKing, Square bKing) {
        for (Color perspective : {WHITE, BLACK}) {
            unsigned int index = getInputIndex(perspective, pieceColor, pieceType, sq, ((perspective == WHITE) ? wKing : flipSquare(bKing)));
#ifdef AVX2
            for (int i = 0; i < chunkNum; i++) {
                const int offset = i * regWidth;
                __m256i ac = _mm256_load_si256((__m256i *) &hiddenLayer[perspective][offset]);
                __m256i we = _mm256_load_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset]);
                __m256i sum = _mm256_add_epi16(ac, we);
                _mm256_store_si256((__m256i *) &hiddenLayer[perspective][offset], sum);
            }
#else
            for (int i = 0; i < L_1_SIZE; i++) {
                hiddenLayer[perspective][i] += L_0_WEIGHTS[index * L_1_SIZE + i];
            }
#endif
        }
    }

    // Removes a feature and forward propagates it to L_1.
    void Accumulator::removeFeature(Color pieceColor, PieceType pieceType, Square sq, Square wKing, Square bKing) {
        for (Color perspective : {WHITE, BLACK}) {
            unsigned int index = getInputIndex(perspective, pieceColor, pieceType, sq, ((perspective == WHITE) ? wKing : flipSquare(bKing)));
#ifdef AVX2
            for (int i = 0; i < chunkNum; i++) {

                const int offset = i * regWidth;

                __m256i ac = _mm256_load_si256((__m256i *) &hiddenLayer[perspective][offset]);
                __m256i we = _mm256_load_si256((__m256i *) &L_0_WEIGHTS[index * L_1_SIZE + offset]);
                __m256i sum = _mm256_sub_epi16(ac, we);
                _mm256_store_si256((__m256i *) &hiddenLayer[perspective][offset], sum);
            }
#else
            for (int i = 0; i < L_1_SIZE; i++) {
                hiddenLayer[perspective][i] -= L_0_WEIGHTS[index * L_1_SIZE + i];
            }
#endif
        }
    }

    // Forward propagates L_1 to L_2.
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

        // Scales back the output with the quantization scales.
        return output * 200 / (255 * 255);
    }

    void init() {

        // Loads values from the embedded net.
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
} // namespace NNUE
