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

// The concept of this class is from https://www.chessprogramming.org/Encoding_Moves
#ifndef CHESS_MOVE_H
#define CHESS_MOVE_H

#include "utils.h"
#include "constants.h"

constexpr unsigned int PROMO_FLAG = 0x8;    // 0b1000
constexpr unsigned int CAPTURE_FLAG = 0x4;  // 0b0100
constexpr unsigned int SPECIAL1_FLAG = 0x2; // 0b0010
constexpr unsigned int SPECIAL2_FLAG = 0x1; // 0b0001

constexpr unsigned int QUIET_MOVE = 0;
constexpr unsigned int CAPTURE = CAPTURE_FLAG;

constexpr unsigned int DOUBLE_PAWN_PUSH = SPECIAL2_FLAG;
constexpr unsigned int EP_CAPTURE = CAPTURE_FLAG | SPECIAL2_FLAG;

constexpr unsigned int PROMO_KNIGHT = PROMO_FLAG;
constexpr unsigned int PROMO_BISHOP = PROMO_FLAG | SPECIAL2_FLAG;
constexpr unsigned int PROMO_ROOK = PROMO_FLAG | SPECIAL1_FLAG;
constexpr unsigned int PROMO_QUEEN = PROMO_FLAG | SPECIAL1_FLAG | SPECIAL2_FLAG;

constexpr unsigned int PROMO_CAPTURE_KNIGHT = CAPTURE_FLAG | PROMO_FLAG;
constexpr unsigned int PROMO_CAPTURE_BISHOP = CAPTURE_FLAG | PROMO_FLAG | SPECIAL2_FLAG;
constexpr unsigned int PROMO_CAPTURE_ROOK = CAPTURE_FLAG | PROMO_FLAG | SPECIAL1_FLAG;
constexpr unsigned int PROMO_CAPTURE_QUEEN = CAPTURE_FLAG | PROMO_FLAG | SPECIAL1_FLAG | SPECIAL2_FLAG;

constexpr unsigned int KING_CASTLE = SPECIAL1_FLAG;
constexpr unsigned int QUEEN_CASTLE = SPECIAL1_FLAG | SPECIAL2_FLAG;


class Move {
public:
    constexpr Move(Square from, Square to, unsigned int flags, Piece capturedPiece) {
        data = ((encodePiece(capturedPiece)) << 16 | (flags & 0xf) << 12 | (from & 0x3f) << 6 | (to & 0x3f));
    }

    constexpr Move(Square from, Square to, unsigned int flags) {
        data = ((encodePiece({})) << 16 | (flags & 0xf) << 12 | (from & 0x3f) << 6 | (to & 0x3f));
    }

    constexpr Move() { data = 0; }

    constexpr Square getTo() const { return Square(data & 0x3f); }

    constexpr Square getFrom() const { return Square((data >> 6) & 0x3f); }

    constexpr Piece getCapturedPiece() const { return decodePiece(data >> 16); }

    constexpr bool isFlag(unsigned int flag) const { return ((data >> 12) & 0xf) & flag; }

    constexpr bool equalFlag(unsigned int flag) const { return ((data >> 12) & 0xf) == (flag & 0xf); }

    constexpr bool isNull() const { return data == 0; }

    bool operator==(Move) const;

    bool operator!=(Move) const;

    bool isCapture() const;

    bool isPromo() const;

    bool isSpecial1() const;

    bool isSpecial2() const;

    bool isQuiet() const;

    explicit operator bool() const;

    std::string str() const;

private:
    unsigned int data;
};

#endif //CHESS_MOVE_H
