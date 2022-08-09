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

#ifndef BLACKCORE_POSITION_H
#define BLACKCORE_POSITION_H


#include "bitboard.h"
#include "utils.h"

class Position {
public:
    Color stm;
    Square epSquare;
    unsigned char castlingRights{};

    constexpr Piece pieceAt(Square square) { return board[square]; }

    // This will be mostly used with constant color and type so this will result a nicer code
    // pieces<{ROOK, WHITE}>() --> pieces<WHITE, ROOK>()
    template<Color color, PieceType type>
    constexpr Bitboard Pieces() { return pieceBB[type] & allPieceBB[color]; }

    template<Color color>
    constexpr Bitboard Us() { return allPieceBB[color]; }

    template<Color color>
    constexpr Bitboard Enemy() { return allPieceBB[EnemyColor<color>()]; }

    template<Color color>
    constexpr Bitboard EnemyOrEmpty() { return ~Us<color>(); }

    void display();

    void loadPositionFromFen(const std::string &fen);

    Position();

    Position(const std::string &fen);

private:

    void clearSquare(Square square);

    void setSquare(Square square, Piece piece);

    void clearPosition();

    Piece board[64];

    Bitboard pieceBB[6], allPieceBB[2];

};


#endif //BLACKCORE_POSITION_H
