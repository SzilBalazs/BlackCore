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

#include "bitboard.h"

Bitboard bitMasks[64], pawnMasks[64][2], knightMasks[64], kingMasks[64];

void initBitboard() {

    for (Square square = A1; square < 64; square += 1) {
        bitMasks[square] = 1 << square;
    }

    for (unsigned int sq = 0; sq < 64; sq++) {
        pawnMasks[sq][WHITE] = step<NORTH_WEST>(bitMasks[sq]) | step<NORTH_EAST>(bitMasks[sq]);
        pawnMasks[sq][BLACK] = step<SOUTH_WEST>(bitMasks[sq]) | step<SOUTH_EAST>(bitMasks[sq]);
    }

    for (unsigned int sq = 0; sq < 64; sq++) {
        knightMasks[sq] =
                step<NORTH>(step<NORTH_WEST>(bitMasks[sq])) | step<NORTH>(step<NORTH_EAST>(bitMasks[sq])) |
                step<WEST>(step<NORTH_WEST>(bitMasks[sq])) | step<EAST>(step<NORTH_EAST>(bitMasks[sq])) |
                step<SOUTH>(step<SOUTH_WEST>(bitMasks[sq])) | step<SOUTH>(step<SOUTH_EAST>(bitMasks[sq])) |
                step<WEST>(step<SOUTH_WEST>(bitMasks[sq])) | step<EAST>(step<SOUTH_EAST>(bitMasks[sq]));
    }

    for (unsigned int sq = 0; sq < 64; sq++) {
        kingMasks[sq] =
                step<NORTH>(bitMasks[sq]) | step<NORTH_WEST>(bitMasks[sq]) | step<WEST>(bitMasks[sq]) |
                step<NORTH_EAST>(bitMasks[sq]) |
                step<SOUTH>(bitMasks[sq]) | step<SOUTH_WEST>(bitMasks[sq]) | step<EAST>(bitMasks[sq]) |
                step<SOUTH_EAST>(bitMasks[sq]);
    }

}
