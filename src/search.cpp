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

#include "search.h"
#include "eval.h"

Score quiescence(Position &pos, Score alpha, Score beta, Ply ply) {
    return eval(pos);
}

Score search(Position &pos, Depth depth, Score alpha, Score beta, Ply ply) {

    if (depth == 0) return eval(pos);

    Color color = pos.getSideToMove();

    Move moves[200];
    Move *movesEnd = generateMoves(pos, moves);

    unsigned int moveCnt = movesEnd - moves;

    if (moveCnt == 0) {
        Bitboard checkers = getAttackers(pos, pos.pieces<KING>(color).lsb());
        if (checkers) {
            return -MATE_VALUE + ply;
        } else {
            return DRAW_VALUE;
        }
    }

    Move bestMove;

    for (int i = 0; i < moveCnt; i++) {

        pos.makeMove(moves[i]);

        Score score = -search(pos, depth - 1, -beta, -alpha, ply + 1);

        pos.undoMove(moves[i]);

        if (score >= beta) {
            return beta;
        }

        if (score > alpha) {
            alpha = score;
            bestMove = moves[i];
        }

    }

    return alpha;
}