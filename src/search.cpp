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
#include <iostream>
#include "eval.h"

Score quiescence(Position &pos, Score alpha, Score beta, Ply ply) {
    Score staticEval = eval(pos);

    if (staticEval >= beta) {
        return beta;
    }

    if (staticEval > alpha) {
        alpha = staticEval;
    }

    MoveList moves = {pos, false};

    while (!moves.empty()) {

        Move m = moves.nextMove();

        pos.makeMove(m);

        Score score = -quiescence(pos, -beta, -alpha, ply + 1);

        pos.undoMove(m);

        if (score >= beta) {
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

Score search(Position &pos, Depth depth, Score alpha, Score beta, Ply ply) {

    if (depth == 0) return quiescence(pos, alpha, beta, ply);

    Color color = pos.getSideToMove();

    MoveList moves = {pos, false};

    if (moves.count == 0) {
        Bitboard checkers = getAttackers(pos, pos.pieces<KING>(color).lsb());
        if (checkers) {
            return -MATE_VALUE + ply;
        } else {
            return DRAW_VALUE;
        }
    }

    Move bestMove;

    while (!moves.empty()) {

        Move m = moves.nextMove();

        pos.makeMove(m);

        Score score = -search(pos, depth - 1, -beta, -alpha, ply + 1);

        pos.undoMove(m);

        if (score >= beta) {
            return beta;
        }

        if (score > alpha) {
            alpha = score;
            bestMove = m;
        }

    }

    // Root node
    if (ply == 0) {
        std::cout << "bestmove " << bestMove << std::endl;
    }

    return alpha;
}