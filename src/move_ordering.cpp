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

#include "move_ordering.h"
#include "tt.h"

constexpr Score mmvlva[6][6] = {
        //       KING   PAWN     KNIGHT    BISHOP    ROOK      QUEEN
        {0, 0,    0,    0,    0,    0},     // KING
        {0, 8004, 8104, 8204, 8304, 8404},  // PAWN
        {0, 8003, 8103, 8203, 8303, 8403},  // KNIGHT
        {0, 8002, 8102, 8202, 8302, 8402},  // BISHOP
        {0, 8001, 8101, 8201, 8301, 8401},  // ROOK
        {0, 8000, 8100, 8200, 8300, 8400},  // QUEEN
};

Score scoreMove(const Position &pos, Move m) {
    Square from = m.getFrom();
    Square to = m.getTo();
    if (m == getHashMove(pos.getHash())) {
        return 10000;
    } else if (m.isCapture()) {
        return mmvlva[pos.pieceAt(from).type][pos.pieceAt(to).type];
    }
    return 0;
}