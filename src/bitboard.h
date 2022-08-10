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

#ifndef BLACKCORE_BITBOARD_H
#define BLACKCORE_BITBOARD_H

#include <cassert>
#include "constants.h"

struct Bitboard {

    U64 bb;

    constexpr Bitboard(U64 value) { bb = value; }

    inline Bitboard(Square square);

    constexpr Bitboard() { bb = 0; }

    constexpr bool get(Square square) const { return (bb >> square) & 1; }

    constexpr void set(Square square) { bb |= 1ULL << square; }

    constexpr void clear(Square square) { bb &= ~(1ULL << square); }

    constexpr int popCount() const { return __builtin_popcountll(bb); }

    constexpr Square lsb() const { return Square(__builtin_ctzll(bb)); }

    constexpr Square popLsb() {
        Square square = lsb();
        clear(square);
        return square;
    }

    constexpr Bitboard operator*(Bitboard a) const { return bb * a.bb; }

    constexpr bool operator==(Bitboard a) const { return bb == a.bb; }

    constexpr bool operator!=(Bitboard a) const { return bb != a.bb; }

    constexpr Bitboard operator+(Bitboard a) const { return bb + a.bb; }

    constexpr Bitboard operator-(Bitboard a) const { return bb - a.bb; }

    constexpr Bitboard operator&(Bitboard a) const { return bb & a.bb; }

    constexpr Bitboard operator|(Bitboard a) const { return bb | a.bb; }

    constexpr Bitboard operator^(Bitboard a) const { return bb ^ a.bb; }

    constexpr Bitboard operator~() const { return ~bb; }

    constexpr Bitboard operator<<(const unsigned int a) const { return bb << a; }

    constexpr Bitboard operator>>(const unsigned int a) const { return bb >> a; }

    constexpr void operator&=(Bitboard a) { bb &= a.bb; }

    constexpr void operator|=(Bitboard a) { bb |= a.bb; }

    constexpr void operator^=(Bitboard a) { bb ^= a.bb; }

    constexpr void operator<<=(const unsigned int a) { bb <<= a; }

    constexpr void operator>>=(const unsigned int a) { bb >>= a; }

    constexpr explicit operator bool() const { return bb; }

    constexpr explicit operator U64() const { return bb; }
};

struct Magic {
    Bitboard *ptr;
    Bitboard mask;
    Bitboard magic;
    unsigned int shift;
};

extern Magic rookMagics[64];
extern Magic bishopMagics[64];
extern Bitboard rookAttackTable[102400];
extern Bitboard bishopAttackTable[5248];

constexpr Bitboard fileA = 0x101010101010101;
constexpr Bitboard fileB = fileA << 1;
constexpr Bitboard fileC = fileA << 2;
constexpr Bitboard fileD = fileA << 3;
constexpr Bitboard fileE = fileA << 4;
constexpr Bitboard fileF = fileA << 5;
constexpr Bitboard fileG = fileA << 6;
constexpr Bitboard fileH = fileA << 7;
constexpr Bitboard rank1 = 0xff;
constexpr Bitboard rank2 = rank1 << (1 * 8);
constexpr Bitboard rank3 = rank1 << (2 * 8);
constexpr Bitboard rank4 = rank1 << (3 * 8);
constexpr Bitboard rank5 = rank1 << (4 * 8);
constexpr Bitboard rank6 = rank1 << (5 * 8);
constexpr Bitboard rank7 = rank1 << (6 * 8);
constexpr Bitboard rank8 = rank1 << (7 * 8);
constexpr Bitboard notFileA = ~fileA;
constexpr Bitboard notFileH = ~fileH;

extern Bitboard bitMasks[64];
extern Bitboard pawnMasks[64][2];
extern Bitboard knightMasks[64];
extern Bitboard kingMasks[64];
extern Bitboard fileMasks[64];
extern Bitboard rankMasks[64];
extern Bitboard rookMasks[64];
extern Bitboard bishopMasks[64];

void initBitboard();

inline Bitboard::Bitboard(Square square) { bb = bitMasks[square].bb; }

inline Bitboard pawnMask(Square square, Color color) { return pawnMasks[square][color]; }

inline Bitboard knightMask(Square square) { return knightMasks[square]; }

inline Bitboard kingMask(Square square) { return kingMasks[square]; }

inline Bitboard fileMask(Square square) { return fileMasks[square]; }

inline Bitboard rankMask(Square square) { return rankMasks[square]; }

inline Bitboard rookMask(Square square) { return rookMasks[square]; }

inline Bitboard bishopMask(Square square) { return bishopMasks[square]; }

inline Bitboard rookAttacks(Square square, Bitboard occ) {
    Magic &m = rookMagics[square];
    return m.ptr[(((occ & m.mask) * m.magic) >> (64 - m.shift)).bb];
}

inline Bitboard bishopAttacks(Square square, Bitboard occ) {
    Magic &m = bishopMagics[square];
    return m.ptr[(((occ & m.mask) * m.magic) >> (64 - m.shift)).bb];
}

inline Bitboard queenAttacks(Square square, Bitboard occ) {
    return rookAttacks(square, occ) | bishopAttacks(square, occ);
}

template<PieceType type>
inline Bitboard pieceAttacks(Square square, Bitboard occupied) {
    assert(type != PAWN);
    switch (type) {
        case KNIGHT:
            return knightMask(square);
        case BISHOP:
            return bishopAttacks(square, occupied);
        case ROOK:
            return rookAttacks(square, occupied);
        case QUEEN:
            return queenAttacks(square, occupied);
        case KING:
            return kingMask(square);
    }
}

template<Direction direction>
constexpr Bitboard step(Bitboard b) {
    switch (direction) {
        case NORTH:
            return b << 8;
        case SOUTH:
            return b >> 8;
        case NORTH_WEST:
            return (b & notFileA) << 7;
        case WEST:
            return (b & notFileA) >> 1;
        case SOUTH_WEST:
            return (b & notFileA) >> 9;
        case NORTH_EAST:
            return (b & notFileH) << 9;
        case EAST:
            return (b & notFileH) << 1;
        case SOUTH_EAST:
            return (b & notFileH) >> 7;
    }
}

template<Direction direction>
constexpr Bitboard slide(Square square) {
    Bitboard result;
    Bitboard temp = {square};
    while (temp) {
        temp = step<direction>(temp);
        result |= temp;
    }
    return result;
}

template<Direction direction>
constexpr Bitboard slide(Square square, Bitboard occupied) {
    Bitboard result;
    Bitboard temp = {square};
    while (temp) {
        temp = step<direction>(temp);
        result |= temp;
        if (occupied.get(temp.lsb())) break;
    }
    return result;
}

Bitboard slidingAttacks(Square square, Bitboard occupied, PieceType type);

void findMagics(Bitboard *attackTable, Magic *magics, PieceType type);

#endif //BLACKCORE_BITBOARD_H
