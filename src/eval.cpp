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

#include "eval.h"

constexpr Score PIECE_VALUES[6] = {0, 100, 300, 300, 500, 900};

Score eval(const Position &pos) {
    Score whiteEval = 0;
    whiteEval += pos.pieces<WHITE, PAWN>().popCount() * PIECE_VALUES[PAWN];
    whiteEval += pos.pieces<WHITE, KNIGHT>().popCount() * PIECE_VALUES[KNIGHT];
    whiteEval += pos.pieces<WHITE, BISHOP>().popCount() * PIECE_VALUES[BISHOP];
    whiteEval += pos.pieces<WHITE, ROOK>().popCount() * PIECE_VALUES[ROOK];
    whiteEval += pos.pieces<WHITE, QUEEN>().popCount() * PIECE_VALUES[QUEEN];

    Score blackEval = 0;
    blackEval += pos.pieces<BLACK, PAWN>().popCount() * PIECE_VALUES[PAWN];
    blackEval += pos.pieces<BLACK, KNIGHT>().popCount() * PIECE_VALUES[KNIGHT];
    blackEval += pos.pieces<BLACK, BISHOP>().popCount() * PIECE_VALUES[BISHOP];
    blackEval += pos.pieces<BLACK, ROOK>().popCount() * PIECE_VALUES[ROOK];
    blackEval += pos.pieces<BLACK, QUEEN>().popCount() * PIECE_VALUES[QUEEN];

    if (pos.getSideToMove() == WHITE)
        return whiteEval - blackEval;
    else
        return blackEval - whiteEval;
}