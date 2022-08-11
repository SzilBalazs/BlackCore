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

#ifndef BLACKCORE_CONSTANTS_H
#define BLACKCORE_CONSTANTS_H

#include <string>

typedef unsigned long long U64;

const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

const unsigned char WK_MASK = 1;
const unsigned char WQ_MASK = 2;
const unsigned char BK_MASK = 4;
const unsigned char BQ_MASK = 8;

enum Square : int {
    A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7,
    A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15,
    A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23,
    A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31,
    A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39,
    A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47,
    A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55,
    A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63,
    NULL_SQUARE = 64
};

inline Square operator+(Square &a, int b) { return Square(int(a) + b); }

inline Square operator-(Square &a, int b) { return Square(int(a) - b); }

inline Square operator+=(Square &a, int b) { return a = a + b; }

inline Square operator-=(Square &a, int b) { return a = a - b; }

inline std::istream &operator>>(std::istream &is, Square &square) {
    std::string s;
    is >> s;

    if (s[0] == '-') {
        square = NULL_SQUARE;
    } else if ('a' <= s[0] && s[0] <= 'z') {
        square = Square((s[0] - 'a') + (s[1] - '1') * 8);
    } else if ('A' <= s[0] && s[0] <= 'Z') {
        square = Square((s[0] - 'A') + (s[1] - '1') * 8);
    }

    return is;
}

enum LineType : int {
    HORIZONTAL = 0, VERTICAL = 1, DIAGONAL = 2, ANTI_DIAGONAL = 3
};

enum Direction : int {
    NORTH = 8, WEST = -1, SOUTH = -8, EAST = 1, NORTH_EAST = 9, NORTH_WEST = 7, SOUTH_WEST = -9, SOUTH_EAST = -7
};

constexpr Direction DIRECTIONS[8] = {NORTH, WEST, NORTH_EAST, NORTH_WEST, SOUTH, EAST, SOUTH_WEST, SOUTH_EAST};

constexpr Direction opposite(Direction direction) { return Direction(-direction); }

constexpr Direction operator-(Direction direction) { return opposite(direction); }

enum PieceType {
    PIECE_EMPTY = 6, KING = 0, PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5
};

enum Color {
    COLOR_EMPTY = 2, WHITE = 0, BLACK = 1
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

    constexpr bool isNull() const { return type == PIECE_EMPTY || color == COLOR_EMPTY; }
};

#endif //BLACKCORE_CONSTANTS_H
