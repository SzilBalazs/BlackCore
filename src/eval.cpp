// BlackCore is a chess engine
// Copyright (c) 2023 SzilBalazs
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

#include "eval.h"

Score distance(Square a, Square b) {
    return std::max(std::abs((a & 7) - (b & 7)), std::abs((a >> 3) - (b >> 3)));
}

Score distanceToEdge(Square a) {
    int rank = a >> 3;
    int file = a & 7;
    return std::min(rank, 7 - rank) + std::min(file, 7 - file);
}

Score evaluateEndgame(const Position &pos) {
    Bitboard occ = pos.occupied();
    if (occ.popCount() == 3) {
        int rook = pos.pieces<ROOK>().popCount();
        int queen = pos.pieces<QUEEN>().popCount();

        if (rook || queen) {
            Color wSide = pos.pieceAt((occ ^ pos.pieces<KING>()).lsb()).color;
            Color lSide = EnemyColor(wSide);
            Square wKing = pos.pieces(wSide, KING).lsb();
            Square lKing = pos.pieces(lSide, KING).lsb();

            Score score = TB_WORST_WIN + 100 - distanceToEdge(lKing) - distance(wKing, lKing);
            if (lSide == pos.getSideToMove()) score *= -1;
            return score;
        }
    }

    return UNKNOWN_SCORE;
}

// Evaluates a position
Score evaluate(const Position &pos) {
    if (pos.isDraw())
        return DRAW_VALUE;

    Score egScore = evaluateEndgame(pos);

    if (egScore == UNKNOWN_SCORE)
        return pos.getState()->accumulator.forward(pos.getSideToMove());
    else
        return egScore;
}
