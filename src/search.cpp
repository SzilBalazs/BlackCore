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

#include <iostream>
#include <chrono>

#include "search.h"
#include "tt.h"
#include "eval.h"

uint64_t nodeCount = 0;
std::chrono::steady_clock::time_point searchBegin;

Score quiescence(Position &pos, Score alpha, Score beta, Ply ply) {
    nodeCount++;

    Score staticEval = eval(pos);

    if (staticEval >= beta) {
        return beta;
    }

    if (staticEval > alpha) {
        alpha = staticEval;
    }

    MoveList moves = {pos, true};

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

    nodeCount++;

    Score ttScore = ttProbe(pos.getHash(), depth, alpha, beta);
    if (ttScore != UNKNOWN_SCORE) return ttScore;

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
    EntryFlag ttFlag = ALPHA;

    while (!moves.empty()) {

        Move m = moves.nextMove();

        pos.makeMove(m);

        Score score = -search(pos, depth - 1, -beta, -alpha, ply + 1);

        pos.undoMove(m);

        if (score >= beta) {
            ttSave(pos.getHash(), depth, beta, BETA, m);
            return beta;
        }

        if (score > alpha) {
            alpha = score;
            bestMove = m;
            ttFlag = EXACT;
        }

    }

    ttSave(pos.getHash(), depth, alpha, ttFlag, bestMove);

    return alpha;
}

std::string getPvLine(Position &pos) {
    Move m = getHashMove(pos.getHash());
    if (m) {
        pos.makeMove(m);
        std::string str = m.str() + " " + getPvLine(pos);
        pos.undoMove(m);
        return str;
    } else {
        return "";
    }
}

Score searchRoot(Position &pos, Depth depth, bool uci) {

    Score score = search(pos, depth, -INF_SCORE, INF_SCORE, 0);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    uint64_t millis = std::chrono::duration_cast<std::chrono::milliseconds>(end - searchBegin).count();
    uint64_t nps = millis == 0 ? 0 : nodeCount * 1000 / millis;

    std::string pvLine = getPvLine(pos);

    if (uci) {
        std::cout << "info depth " << depth << " nodes " << nodeCount << " score " << score << " time " << millis
                  << " nps " << nps << " pv "
                  << pvLine
                  << std::endl;
    }

    return score;
}

void iterativeDeepening(Position &pos, Depth depth, bool uci) {
    nodeCount = 0;

    searchBegin = std::chrono::steady_clock::now();

    for (Depth currDepth = 1; currDepth <= depth; currDepth++) {
        searchRoot(pos, currDepth, uci);
    }
    std::cout << "bestmove " << getHashMove(pos.getHash()) << std::endl;
}