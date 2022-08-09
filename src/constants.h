#ifndef BLACKCORE_CONSTANTS_H
#define BLACKCORE_CONSTANTS_H

typedef unsigned long long U64;

enum Square : int {
    A1=0, B1=1, C1=2, D1=3, E1=4, F1=5, G1=6, H1=7,
    A2=8, B2=9, C2=10, D2=11, E2=12, F2=13, G2=14, H2=15,
    A3=16, B3=17, C3=18, D3=19, E3=20, F3=21, G3=22, H3=23,
    A4=24, B4=25, C4=26, D4=27, E4=28, F4=29, G4=30, H4=31,
    A5=32, B5=33, C5=34, D5=35, E5=36, F5=37, G5=38, H5=39,
    A6=40, B6=41, C6=42, D6=43, E6=44, F6=45, G6=46, H6=47,
    A7=48, B7=49, C7=50, D7=51, E7=52, F7=53, G7=54, H7=55,
    A8=56, B8=57, C8=58, D8=59, E8=60, F8=61, G8=62, H8=63,
    NULL_SQUARE=64
};

inline Square operator+=(Square& a, int b) { return a = Square(int(a)+b); }
inline Square operator-=(Square& a, int b) { return a = Square(int(a)-b); }

enum Direction {
    NORTH = 0, WEST = 1, SOUTH = 2, EAST = 3, NORTH_EAST = 4, NORTH_WEST = 5, SOUTH_WEST = 6, SOUTH_EAST = 7
};

enum PieceType {
    PIECE_EMPTY=6, KING=0, PAWN=1, KNIGHT=2, BISHOP=3, ROOK=4, QUEEN=5
};

enum Color {
    COLOR_EMPTY=2, WHITE=0, BLACK=1
};

struct Piece {
    PieceType type;
    Color color;

    constexpr Piece() {
        type = PIECE_EMPTY;
        color = COLOR_EMPTY;
    }

    constexpr Piece(PieceType t, Color c) {
        type = t;
        color = c;
    }

};

#endif //BLACKCORE_CONSTANTS_H
