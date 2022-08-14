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
#include "eval.h"

Score scoreMove(const Position &pos, Move m) {
    Square from = m.getFrom();
    Square to = m.getTo();
    if (m == getHashMove(pos.getHash())) {
        return 10000;
    } else if (m.isCapture()) {
        return PIECE_VALUES[pos.pieceAt(to).type] - PIECE_VALUES[pos.pieceAt(from).type];
    }
    return 0;
}