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

#include "bitboard.h"
#include "utils.h"
#include <cstring>
#include <iostream>

// Lookup tables generated when the initBitboard() function gets called.
Bitboard bitMasks[64], pawnMasks[64][2], knightMasks[64], kingMasks[64], fileMasks[64], rankMasks[64], rookMasks[64],
        diagonalMasks[64], antiDiagonalMasks[64], bishopMasks[64],
        rookAttackTable[102400], bishopAttackTable[5248], commonRay[64][64], adjacentFileMasks[64], adjacentNorthMasks[64],
        adjacentSouthMasks[64];
LineType lineType[64][64];

/*
 * Initializes values, regarding bitboard. Must be called
 * before calling the move generator.
 */
void initBitboard() {

    for (Square sq = A1; sq < 64; sq += 1) {
        bitMasks[sq] = 1ULL << sq;

        pawnMasks[sq][WHITE] = step<NORTH_WEST>(bitMasks[sq]) | step<NORTH_EAST>(bitMasks[sq]);
        pawnMasks[sq][BLACK] = step<SOUTH_WEST>(bitMasks[sq]) | step<SOUTH_EAST>(bitMasks[sq]);

        knightMasks[sq] =
                step<NORTH>(step<NORTH_WEST>(bitMasks[sq])) | step<NORTH>(step<NORTH_EAST>(bitMasks[sq])) |
                step<WEST>(step<NORTH_WEST>(bitMasks[sq])) | step<EAST>(step<NORTH_EAST>(bitMasks[sq])) |
                step<SOUTH>(step<SOUTH_WEST>(bitMasks[sq])) | step<SOUTH>(step<SOUTH_EAST>(bitMasks[sq])) |
                step<WEST>(step<SOUTH_WEST>(bitMasks[sq])) | step<EAST>(step<SOUTH_EAST>(bitMasks[sq]));

        kingMasks[sq] =
                step<NORTH>(bitMasks[sq]) | step<NORTH_WEST>(bitMasks[sq]) | step<WEST>(bitMasks[sq]) |
                step<NORTH_EAST>(bitMasks[sq]) |
                step<SOUTH>(bitMasks[sq]) | step<SOUTH_WEST>(bitMasks[sq]) | step<EAST>(bitMasks[sq]) |
                step<SOUTH_EAST>(bitMasks[sq]);

        fileMasks[sq] = slide<NORTH>(sq) | slide<SOUTH>(sq);

        rankMasks[sq] = slide<WEST>(sq) | slide<EAST>(sq);

        rookMasks[sq] = fileMasks[sq] | rankMasks[sq];

        diagonalMasks[sq] = slide<NORTH_EAST>(sq) | slide<SOUTH_WEST>(sq);

        antiDiagonalMasks[sq] = slide<NORTH_WEST>(sq) | slide<SOUTH_EAST>(sq);

        bishopMasks[sq] = diagonalMasks[sq] | antiDiagonalMasks[sq];
    }

    for (Square sq = A1; sq < 64; sq += 1) {
        unsigned int file = squareToFile(sq);

        adjacentNorthMasks[sq] = slide<NORTH>(sq) | (file != 0 ? slide<NORTH>(sq + WEST) : 0) |
                                 (file != 7 ? slide<NORTH>(sq + EAST) : 0);
        adjacentSouthMasks[sq] = slide<SOUTH>(sq) | (file != 0 ? slide<SOUTH>(sq + WEST) : 0) |
                                 (file != 7 ? slide<SOUTH>(sq + EAST) : 0);
        adjacentFileMasks[sq] =
                ~fileMasks[sq] & (adjacentNorthMasks[sq] | adjacentSouthMasks[sq] | step<WEST>(sq) | step<EAST>(sq));

        // Calculates the common ray and the line type of the shortest path between sq and sq2.
        for (Square sq2 = A1; sq2 < 64; sq2 += 1) {
            if (sq == sq2)
                continue;
            for (Direction dir : DIRECTIONS) {
                Bitboard value = slide(dir, sq) & slide(-dir, sq2);

                if (value) {
                    commonRay[sq][sq2] = value;
                    LineType type = HORIZONTAL;
                    switch (dir) {
                        case NORTH:
                        case SOUTH:
                            type = HORIZONTAL;
                            break;
                        case WEST:
                        case EAST:
                            type = VERTICAL;
                            break;
                        case NORTH_EAST:
                        case SOUTH_WEST:
                            type = DIAGONAL;
                            break;
                        case NORTH_WEST:
                        case SOUTH_EAST:
                            type = ANTI_DIAGONAL;
                            break;
                    }
                    lineType[sq][sq2] = type;
                    break;
                }
            }
        }
    }

    /*
     * Initializes magic bitboards, which are used for generating sliding moves.
     * For more information: https://www.chessprogramming.org/Magic_Bitboards
     */
    initMagic(rookMagics, ROOK);
    initMagic(bishopMagics, BISHOP);
}

// Slow naive function of getting the attacked squares of a sliding piece.
Bitboard slidingAttacks(Square square, Bitboard occupied, PieceType type) {
    assert((type == ROOK) || (type == BISHOP));
    switch (type) {
        case ROOK:
            return slide<NORTH>(square, occupied) | slide<SOUTH>(square, occupied) | slide<WEST>(square, occupied) |
                   slide<EAST>(square, occupied);
        case BISHOP:
            return slide<NORTH_WEST>(square, occupied) | slide<NORTH_EAST>(square, occupied) |
                   slide<SOUTH_WEST>(square, occupied) | slide<SOUTH_EAST>(square, occupied);
        default:
            return {};
    }
}

// Initializes magic bitboards.
void initMagic(const Magic *magics, PieceType type) {
    assert((type == ROOK) || (type == BISHOP));
    Bitboard occupied[4096], attacked[4096];

    for (Square square = A1; square < 64; square += 1) {
        const Magic &magic = magics[square];

        unsigned int length = 0;
        Bitboard occ = 0;
        do {
            occupied[length] = occ;
            attacked[length] = slidingAttacks(square, occ, type);

            length++;
            occ = (occ - magic.mask) & magic.mask;
        } while (occ != 0);

        for (unsigned int i = 0; i < length; i++) {
            U64 index = getMagicIndex(magic, occupied[i]);

            magic.ptr[index] = attacked[i];
        }
    }
}

// When called it generates magics and outputs them to the console.
void findMagics(Bitboard *attackTable, Magic *magics, PieceType type) {
    assert((type == ROOK) || (type == BISHOP));
    Bitboard occupied[4096], attacked[4096];

    if (type == ROOK)
        std::cout << "Magic rookMagics[64] = {\n";
    else
        std::cout << "Magic bishopMagics[64] = {\n";

    unsigned int length = 0;
    for (Square square = A1; square < 64; square += 1) {
        Bitboard edge = (((rank1 | rank8) & ~rankMasks[square]) | ((fileA | fileH) & ~fileMasks[square]));

        Magic &magic = magics[square];

        magic.mask = slidingAttacks(square, 0, type) & ~edge;
        magic.shift = magic.mask.popCount();

        if (square == A1)
            magic.ptr = attackTable;
        else
            magic.ptr = magics[square - 1].ptr + length;

        // Carry-Ripler trick: https://www.chessprogramming.org/Traversing_Subsets_of_a_Set
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
            if (((magic.magic * magic.mask) >> 56).popCount() < 6)
                continue;

            std::memset(used, 0, sizeof(used));

            bool failed = false;
            for (unsigned int i = 0; i < length; i++) {
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
        if (type == ROOK)
            std::cout << "  {rookAttackTable + " << magic.ptr - rookAttackTable << ", " << BBToHex(magic.mask) << ", "
                      << BBToHex(magic.magic) << ", " << magic.shift << "},\n";
        else
            std::cout << "  {bishopAttackTable + " << magic.ptr - bishopAttackTable << ", " << BBToHex(magic.mask)
                      << ", "
                      << BBToHex(magic.magic) << ", " << magic.shift << "},\n";
    }

    std::cout << "};\n";
}