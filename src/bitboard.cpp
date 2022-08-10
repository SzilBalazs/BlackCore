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

#include <iostream>
#include <cstring>
#include "bitboard.h"
#include "utils.h"

Bitboard bitMasks[64], pawnMasks[64][2], knightMasks[64], kingMasks[64], fileMasks[64], rankMasks[64], rookMasks[64], bishopMasks[64], rookAttackTable[102400], bishopAttackTable[5248];
Magic rookMagics[64], bishopMagics[64];

void initBitboard() {

    for (Square square = A1; square < 64; square += 1) {
        bitMasks[square] = 1ULL << square;
        pawnMasks[square][WHITE] = step<NORTH_WEST>(bitMasks[square]) | step<NORTH_EAST>(bitMasks[square]);
        pawnMasks[square][BLACK] = step<SOUTH_WEST>(bitMasks[square]) | step<SOUTH_EAST>(bitMasks[square]);

        knightMasks[square] =
                step<NORTH>(step<NORTH_WEST>(bitMasks[square])) | step<NORTH>(step<NORTH_EAST>(bitMasks[square])) |
                step<WEST>(step<NORTH_WEST>(bitMasks[square])) | step<EAST>(step<NORTH_EAST>(bitMasks[square])) |
                step<SOUTH>(step<SOUTH_WEST>(bitMasks[square])) | step<SOUTH>(step<SOUTH_EAST>(bitMasks[square])) |
                step<WEST>(step<SOUTH_WEST>(bitMasks[square])) | step<EAST>(step<SOUTH_EAST>(bitMasks[square]));

        kingMasks[square] =
                step<NORTH>(bitMasks[square]) | step<NORTH_WEST>(bitMasks[square]) | step<WEST>(bitMasks[square]) |
                step<NORTH_EAST>(bitMasks[square]) |
                step<SOUTH>(bitMasks[square]) | step<SOUTH_WEST>(bitMasks[square]) | step<EAST>(bitMasks[square]) |
                step<SOUTH_EAST>(bitMasks[square]);

        fileMasks[square] = slide<NORTH>(square) | slide<SOUTH>(square);

        rankMasks[square] = slide<WEST>(square) | slide<EAST>(square);

        rookMasks[square] = slide<NORTH>(square) | slide<SOUTH>(square) | slide<WEST>(square) | slide<EAST>(square);

        bishopMasks[square] = slide<NORTH_WEST>(square) | slide<NORTH_EAST>(square) | slide<SOUTH_WEST>(square) |
                              slide<SOUTH_EAST>(square);
    }

    findMagics(rookAttackTable, rookMagics, ROOK);
    findMagics(bishopAttackTable, bishopMagics, BISHOP);

}

Bitboard slidingAttacks(Square square, Bitboard occupied, PieceType type) {
    switch (type) {
        case ROOK:
            return slide<NORTH>(square, occupied) | slide<SOUTH>(square, occupied) | slide<WEST>(square, occupied) |
                   slide<EAST>(square, occupied);
        case BISHOP:
            return slide<NORTH_WEST>(square, occupied) | slide<NORTH_EAST>(square, occupied) |
                   slide<SOUTH_WEST>(square, occupied) | slide<SOUTH_EAST>(square, occupied);
        default:
            assert(1);
    }
}

void findMagics(Bitboard *attackTable, Magic *magics, PieceType type) {
    assert((type == ROOK) | (type == BISHOP));
    Bitboard occupied[4096], attacked[4096];

    unsigned int length = 0;
    for (Square square = A1; square < 64; square += 1) {
        Bitboard edge = (((rank1 | rank8) & ~rankMask(square)) | ((fileA | fileH) & ~fileMask(square)));

        Magic &magic = magics[square];

        magic.mask = slidingAttacks(square, 0, type) & ~edge;
        magic.shift = magic.mask.popCount();

        if (square == A1) magic.ptr = attackTable;
        else magic.ptr = magics[square - 1].ptr + length;

        // Carry-Ripler trick for reference check out: https://www.chessprogramming.org/Traversing_Subsets_of_a_Set
        length = 0;
        Bitboard occ = 0;
        do {
            occupied[length] = occ;
            attacked[length] = slidingAttacks(square, occ, type);

            length++;
            occ = (occ - magic.mask) & magic.mask;
        } while (occ != 0);

        Bitboard used[4096];

        while (true) {
            magic.magic = randBB() & randBB() & randBB();
            if (((magic.magic * magic.mask) >> 56).popCount() < 6) continue;

            std::memset(used, 0, sizeof(used));

            bool failed = false;
            for (int i = 0; i < length; i++) {
                U64 index = (((occupied[i] & magic.mask) * magic.magic) >> (64 - magic.shift)).bb;
                if (used[index] == 0) {
                    used[index] = attacked[i];
                    magic.ptr[index] = attacked[i];
                } else if (used[index] != attacked[i]) {
                    failed = true;
                    break;
                }
            }
            if (!failed) {
                break;
            }
        }
    }
}