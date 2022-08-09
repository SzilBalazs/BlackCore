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

#include <iostream>
#include "constants.h"

const unsigned int PROMO_FLAG = 0x8;    // 0b1000
const unsigned int CAPTURE_FLAG = 0x4;  // 0b0100
const unsigned int SPECIAL1_FLAG = 0x2; // 0b0010
const unsigned int SPECIAL2_FLAG = 0x1; // 0b0001

class Move {
public:
    Move(Square from, Square to, unsigned int flags, Piece capturedPiece);

    Move(Square from, Square to, unsigned int flags);

    Move();

    void operator=(Move);

    Square getTo() const;

    Square getFrom() const;

    Piece getCapturedPiece() const;

    bool isFlag(unsigned int) const;

    bool operator==(Move) const;

    bool operator!=(Move) const;

    bool isCapture() const;

    bool isPromo() const;

    bool isSpecial1() const;

    bool isSpecial2() const;

    bool isQuiet() const;

    explicit operator bool() const;

    bool isNull() const;

    std::string str() const;

private:
    unsigned int data;
};

#endif //CHESS_MOVE_H
