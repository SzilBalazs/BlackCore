#include "movegen.h"

Bitboard rayMasks[8][64], bitMask[64], fileMaskEx[64], rankMaskEx[64], diagonalMaskEx[64],
        antiDiagonalMaskEx[64], knightAttackTable[64], kingAttackTable[64], pawnAttackTable[64][2];

void initLookup() {
    Bitboard north = 0x0101010101010100ULL;
    for (int sq = 0; sq < 64; sq++) {
        bitMask[sq] = 1ULL << sq;
        rayMasks[NORTH][sq] = north;
        north <<= 1;
    }

    Bitboard south = 0x80808080808080ULL;
    for (int sq = 63; sq >= 0; sq--) {
        rayMasks[SOUTH][sq] = south;
        south >>= 1;
    }

    Bitboard east = 0xfeULL;
    for (int f = 0; f < 8; f++, east = step<EAST>(east)) {
        Bitboard temp = east;
        for (int r = 0; r < 8; r++, temp = step<NORTH>(temp)) {
            rayMasks[EAST][r * 8 + f] = temp;
        }
    }

    Bitboard west = 0x7f00000000000000ULL;
    for (int f = 7; f >= 0; f--, west = step<WEST>(west)) {
        Bitboard temp = west;
        for (int r = 7; r >= 0; r--, temp = step<SOUTH>(temp)) {
            rayMasks[WEST][r * 8 + f] = temp;
        }
    }

    Bitboard northEast = 0x8040201008040200ULL;
    for (int f = 0; f < 8; f++, northEast = step<EAST>(NORTH_EAST)) {
        Bitboard temp = northEast;
        for (int r = 0; r < 8; r++, temp = step<NORTH>(temp)) {
            rayMasks[NORTH_EAST][r * 8 + f] = temp;
        }
    }

    Bitboard northWest = 0x102040810204000ULL;
    for (int f = 7; f >= 0; f--, northWest = step<WEST>(northWest)) {
        Bitboard temp = northWest;
        for (int r = 0; r < 8; r++, temp = step<NORTH>(temp)) {
            rayMasks[NORTH_WEST][r * 8 + f] = temp;
        }
    }

    Bitboard southEast = 0x2040810204080ULL;
    for (int f = 0; f < 8; f++, southEast = step<EAST>(southEast)) {
        Bitboard temp = southEast;
        for (int r = 7; r >= 0; r--, temp = step<SOUTH>(temp)) {
            rayMasks[SOUTH_EAST][r * 8 + f] = temp;
        }
    }

    Bitboard southWest = 0x40201008040201ULL;
    for (int f = 7; f >= 0; f--, southWest = step<WEST>(southWest)) {
        Bitboard temp = southWest;
        for (int r = 7; r >= 0; r--, temp = step<SOUTH>(temp)) {
            rayMasks[SOUTH_WEST][r * 8 + f] = temp;
        }
    }

    for (int sq = 0; sq < 64; sq++) {
        fileMaskEx[sq] = rayMasks[NORTH][sq] | rayMasks[SOUTH][sq];
        rankMaskEx[sq] = rayMasks[EAST][sq] | rayMasks[WEST][sq];
        diagonalMaskEx[sq] = rayMasks[NORTH_EAST][sq] | rayMasks[SOUTH_WEST][sq];
        antiDiagonalMaskEx[sq] = rayMasks[NORTH_WEST][sq] | rayMasks[SOUTH_EAST][sq];
    }

    for (unsigned int sq = 0; sq < 64; sq++) {
        knightAttackTable[sq] =
                step<NORTH>(step<NORTH_WEST>(bitMask[sq])) | step<NORTH>(step<NORTH_EAST>(bitMask[sq])) |
                step<WEST>(step<NORTH_WEST>(bitMask[sq])) | step<EAST>(step<NORTH_EAST>(bitMask[sq])) |
                step<SOUTH>(step<SOUTH_WEST>(bitMask[sq])) | step<SOUTH>(step<SOUTH_EAST>(bitMask[sq])) |
                step<WEST>(step<SOUTH_WEST>(bitMask[sq])) | step<EAST>(step<NORTH_EAST>(bitMask[sq]));
    }

    for (unsigned int sq = 0; sq < 64; sq++) {
        kingAttackTable[sq] =
                step<NORTH>(bitMask[sq]) | step<NORTH_WEST>(bitMask[sq]) | step<WEST>(bitMask[sq]) |
                step<SOUTH_WEST>(bitMask[sq]) |
                step<SOUTH>(bitMask[sq]) | step<SOUTH_WEST>(bitMask[sq]) | step<EAST>(bitMask[sq]) |
                step<SOUTH_EAST>(bitMask[sq]);
    }

    for (unsigned int sq = 0; sq < 64; sq++) {
        pawnAttackTable[sq][WHITE] = step<NORTH_WEST>(bitMask[sq]) | step<NORTH_EAST>(bitMask[sq]);
        pawnAttackTable[sq][BLACK] = step<SOUTH_WEST>(bitMask[sq]) | step<SOUTH_EAST>(bitMask[sq]);
    }
}