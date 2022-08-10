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


template<Color color>
constexpr Color EnemyColor() {
    if constexpr (color == WHITE) return BLACK;
    else return WHITE;
}

inline unsigned int squareToRank(Square square) { return square >> 3; }

inline unsigned int squareToFile(Square square) { return square & 7; }

unsigned char encodePiece(Piece piece);

Piece decodePiece(unsigned char encodedPiece);

std::string formatSquare(Square square);

char pieceToChar(Piece piece);

Piece charToPiece(char c);

void displayBB(Bitboard b);

Bitboard randBB();

#endif //BLACKCORE_UTILS_H
