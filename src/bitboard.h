#ifndef BLACKCORE_BITBOARD_H
#define BLACKCORE_BITBOARD_H

#include "constants.h"

struct Bitboard {

    U64 bb;

    constexpr Bitboard(U64 value) { bb = value; }

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
};

constexpr Bitboard fileA = 0x101010101010101;
constexpr Bitboard fileH = 0x8080808080808080;
constexpr Bitboard notFileA = ~fileA;
constexpr Bitboard notFileH = ~fileH;
constexpr Bitboard edge = 0xff818181818181ff;
constexpr Bitboard notEdge = ~edge;

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

#endif //BLACKCORE_BITBOARD_H
