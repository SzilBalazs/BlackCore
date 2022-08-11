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

    constexpr Piece pieceAt(Square square) const { return board[square]; }

    // This will be mostly used with constant color and type so this will result a nicer code
    // pieces<{ROOK, WHITE}>() --> pieces<WHITE, ROOK>()
    template<Color color, PieceType type>
    constexpr Bitboard pieces() const { return pieceBB[type] & allPieceBB[color]; }

    template<PieceType type>
    constexpr Bitboard pieces() const { return pieceBB[type]; }

    template<Color color>
    constexpr Bitboard friendly() const { return allPieceBB[color]; }

    template<Color color>
    constexpr Bitboard enemy() const { return allPieceBB[EnemyColor<color>()]; }

    template<Color color>
    constexpr Bitboard enemyOrEmpty() const { return ~friendly<color>(); }

    inline Bitboard occupied() const { return allPieceBB[WHITE] | allPieceBB[BLACK]; }

    inline Bitboard empty() const { return ~occupied(); }

    inline Color getSideToMove() const { return stm; }

    inline Square getEpSquare() const { return epSquare; }

    inline bool getCastleRight(unsigned char castleRight) { return castleRight & castlingRights; }

    void display();

    void loadPositionFromFen(const std::string &fen);

    Position();

    Position(const std::string &fen);

private:

    void clearSquare(Square square);

    void setSquare(Square square, Piece piece);

    inline void setCastleRight(unsigned char castleRight) { castlingRights |= castleRight; }

    inline void removeCastleRight(unsigned char castleRight) { castlingRights &= ~castleRight; }

    void clearPosition();

    Piece board[64];

    Bitboard pieceBB[6], allPieceBB[2];

    Color stm;
    Square epSquare;
    unsigned char castlingRights;

};


#endif //BLACKCORE_POSITION_H
