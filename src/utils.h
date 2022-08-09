#ifndef BLACKCORE_UTILS_H
#define BLACKCORE_UTILS_H

#include <string>
#include "constants.h"


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


#endif //BLACKCORE_UTILS_H
