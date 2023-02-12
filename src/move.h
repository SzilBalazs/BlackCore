// BlackCore is a chess engine
// Copyright (c) 2022-2023 SzilBalazs
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// The concept of this class is from https://www.chessprogramming.org/Encoding_Moves
#ifndef BLACKCORE_MOVE_H
#define BLACKCORE_MOVE_H

#include "constants.h"
#include "utils.h"

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
    // Initialize the move with a from square, to square, and flags
    constexpr Move(Square from, Square to, unsigned int flags) {
        data = (flags << 12) | (from << 6) | (to);
    }

    // Initialize the move with a from square and to square, default flags
    constexpr Move(Square from, Square to) {
        data = (from << 6) | (to);
    }

    // Default constructor, initialize move as null move
    constexpr Move() = default;

    // Returns the to square of the move
    constexpr Square getTo() const {
        return Square(data & 0x3f);
    }

    // Returns the from square of the move
    constexpr Square getFrom() const {
        return Square((data >> 6) & 0x3f);
    }

    // Returns true if the flag is set in the move
    constexpr bool isFlag(unsigned int flag) const {
        return (data >> 12) & flag;
    }

    // Returns true if the move type is equal to flag
    constexpr bool equalFlag(unsigned int flag) const {
        return (data >> 12) == flag;
    }

    // Returns true if the move is not null
    constexpr bool isOk() const {
        return data != 0;
    }

    // Returns true if the move is a capture
    constexpr bool isCapture() const {
        return isFlag(CAPTURE_FLAG);
    }

    // Returns true if the move is a promotion
    constexpr bool isPromo() const {
        return isFlag(PROMO_FLAG);
    }

    // Returns true if the move has the SPECIAL1_FLAG set
    constexpr bool isSpecial1() const {
        return isFlag(SPECIAL1_FLAG);
    }

    // Returns true if the move has the SPECIAL2_FLAG set
    constexpr bool isSpecial2() const {
        return isFlag(SPECIAL2_FLAG);
    }

    // Returns true if the move is quiet - not a capture.
    constexpr bool isQuiet() const {
        return !isCapture();
    }

    // Explicit conversion to bool, returns true if move is not a null move
    constexpr explicit operator bool() const {
        return isOk();
    }

    // Comparison operator, returns true if moves are equal
    constexpr bool operator==(Move a) const {
        return (data & 0xFFFF) == (a.data & 0xFFFF);
    }

    // Comparison operator, returns true if moves are not equal
    constexpr bool operator!=(Move a) const {
        return (data & 0xFFFF) != (a.data & 0xFFFF);
    }

    // Returns the move in UCI format as a string
    std::string str() const;

private:
    uint16_t data = 0;
};

struct SearchStack {
    Move move, excludedMove;
    Piece movedPiece;
    Score eval = 0;
    Ply ply = 0;
};

constexpr Move MOVE_NULL = Move();

std::ostream &operator<<(std::ostream &os, const Move &move);

#endif //BLACKCORE_MOVE_H
