// main source is https://www.chessprogramming.org/Encoding_Moves
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
