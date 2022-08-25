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

#ifndef BLACKCORE_UTILS_H
#define BLACKCORE_UTILS_H

#include <string>
#include "constants.h"
#include "bitboard.h"

constexpr PieceType indexToType[7] = {KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, PIECE_EMPTY};
constexpr Color indexToColor[3] = {WHITE, BLACK, COLOR_EMPTY};

template<Color color>
constexpr Color EnemyColor() {
    if constexpr (color == WHITE) return BLACK;
    else return WHITE;
}

inline Color EnemyColor(Color color) { return color==WHITE?BLACK:WHITE; }

constexpr unsigned int squareToRank(Square square) { return square >> 3; }

constexpr unsigned int squareToFile(Square square) { return square & 7; }

constexpr Square mirrorSquare(Square square) {
    return Square(56 - square + squareToFile(square));
}

constexpr unsigned char encodePiece(Piece piece) { return (piece.color << 3) | piece.type; }

constexpr Piece decodePiece(unsigned char encodedPiece) {
    return {indexToType[encodedPiece & 7], indexToColor[encodedPiece >> 3]};
}

std::string formatSquare(Square square);

char pieceToChar(Piece piece);

Piece charToPiece(char c);

void displayBB(Bitboard b);

Bitboard randBB();

std::string BBToHex(Bitboard bb);

#endif //BLACKCORE_UTILS_H
