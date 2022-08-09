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

#ifndef BLACKCORE_MOVEGEN_H
#define BLACKCORE_MOVEGEN_H

#include "bitboard.h"


extern Bitboard rayMasks[8][64], bitMask[64], fileMaskEx[64], rankMaskEx[64], diagonalMaskEx[64],
        antiDiagonalMaskEx[64], knightMaskTable[64], kingMaskTable[64], pawnMaskTable[64][2];

void initLookup();

inline Bitboard pawnMask(Square square, Color color) { return pawnMaskTable[square][color]; }

inline Bitboard kingMask(Square square) { return kingMaskTable[square]; }

inline Bitboard knightMask(Square square) { return knightMaskTable[square]; }

#endif //BLACKCORE_MOVEGEN_H
