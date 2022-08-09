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

#include <iostream>
#include "utils.h"
#include "bitboard.h"

const PieceType indexToType[7] = {KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, PIECE_EMPTY};
const Color indexToColor[3] = {WHITE, BLACK, COLOR_EMPTY};

unsigned char encodePiece(Piece piece) {
    return (piece.color << 3) | piece.type;
}

Piece decodePiece(unsigned char encodedPiece) {
    return {indexToType[encodedPiece & 7], indexToColor[encodedPiece >> 3]};
}

std::string formatSquare(Square square) {
    return std::string() + (char) ('a' + (char) square % 8) + (char) ('1' + (char) (square / 8));
}

char pieceToChar(Piece piece) {
    char base;
    switch (piece.type) {
        case PAWN:
            base = 'p';
            break;
        case ROOK:
            base = 'r';
            break;
        case KNIGHT:
            base = 'n';
            break;
        case BISHOP:
            base = 'b';
            break;
        case QUEEN:
            base = 'q';
            break;
        case KING:
            base = 'k';
            break;
        case PIECE_EMPTY:
            base = ' ';
            break;
    }
    if (base != ' ' && piece.color == WHITE) base -= 32;
    return base;
}

Piece charToPiece(char c) {
    Piece piece;

    if ('a' <= c && c <= 'z') {
        piece.color = BLACK;
    } else if ('A' <= c && c <= 'Z') {
        piece.color = WHITE;
        c += 32;
    }

    switch (c) {
        case 'p':
            piece.type = PAWN;
            break;
        case 'r':
            piece.type = ROOK;
            break;
        case 'n':
            piece.type = KNIGHT;
            break;
        case 'b':
            piece.type = BISHOP;
            break;
        case 'q':
            piece.type = QUEEN;
            break;
        case 'k':
            piece.type = KING;
            break;
    }
    return piece;
}

void displayBB(Bitboard b) {
    std::cout << "    A   B   C   D   E   F   G   H  \n";
    for (int i = 8; i >= 1; i--) {
        std::cout << "  +---+---+---+---+---+---+---+---+\n";
        std::cout << i << " |";
        for (int j = 1; j <= 8; j++) {
            std::cout << " " << b.get(Square((i - 1) * 8 + (j - 1))) << " |";
        }
        std::cout << "\n";
    }
    std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
}