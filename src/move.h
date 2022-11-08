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

#include "constants.h"
#include "utils.h"

constexpr unsigned int PROMO_FLAG = 0x8;   // 0b1000
constexpr unsigned int CAPTURE_FLAG = 0x4; // 0b0100
constexpr unsigned int SPECIAL1_FLAG = 0x2;// 0b0010
constexpr unsigned int SPECIAL2_FLAG = 0x1;// 0b0001

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
    constexpr Move(Square from, Square to, unsigned int flags) {
        data = (flags << 12) | (from << 6) | (to);
    }

    constexpr Move(Square from, Square to) {
        data = (from << 6) | (to);
    }

    constexpr Move() = default;

    constexpr Square getTo() const {
        return Square(data & 0x3f);
    }

    constexpr Square getFrom() const {
        return Square((data >> 6) & 0x3f);
    }

    constexpr bool isFlag(unsigned int flag) const {
        return (data >> 12) & flag;
    }

    constexpr bool equalFlag(unsigned int flag) const {
        return (data >> 12) == flag;
    }

    constexpr bool isNull() const {
        return data == 0;
    }

    constexpr bool isCapture() const {
        return isFlag(CAPTURE_FLAG);
    }

    constexpr bool isPromo() const {
        return isFlag(PROMO_FLAG);
    }

    constexpr bool isSpecial1() const {
        return isFlag(SPECIAL1_FLAG);
    }

    constexpr bool isSpecial2() const {
        return isFlag(SPECIAL2_FLAG);
    }

    constexpr bool isQuiet() const {
        return !isCapture();
    }

    constexpr explicit operator bool() const {
        return !isNull();
    }

    constexpr bool operator==(Move a) const {
        return (data & 0xFFFF) == (a.data & 0xFFFF);
    }

    constexpr bool operator!=(Move a) const {
        return (data & 0xFFFF) != (a.data & 0xFFFF);
    }

    std::string str() const;

private:
    uint16_t data = 0;
};

std::ostream &operator<<(std::ostream &os, const Move &move);

#endif//CHESS_MOVE_H
