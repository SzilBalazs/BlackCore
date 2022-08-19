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
#include "timeman.h"
#include "tt.h"
#include "eval.h"
#include "uci.h"

Ply selectiveDepth = 0;

Score quiescence(Position &pos, Score alpha, Score beta, Ply ply) {

    if (shouldEnd()) return UNKNOWN_SCORE;

    if (ply > selectiveDepth)
        selectiveDepth = ply;

    Score staticEval = eval(pos);

    if (staticEval >= beta) {
        return beta;
    }

    if (staticEval > alpha) {
        alpha = staticEval;
    }

    MoveList moves = {pos, ply, true};

    while (!moves.empty()) {

        Move m = moves.nextMove();

        // Delta pruning
        if (m.isPromo() * PIECE_VALUES[QUEEN].mg + PIECE_VALUES[pos.pieceAt(m.getTo()).type].mg +
            staticEval + DELTA_MARGIN < alpha)
            continue;

        pos.makeMove(m);

        Score score = -quiescence(pos, -beta, -alpha, ply + 1);

        pos.undoMove(m);

        if (shouldEnd()) return UNKNOWN_SCORE;

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

    if (shouldEnd()) return UNKNOWN_SCORE;

    if (pos.getMove50() >= 4 && ply > 0 && pos.isRepetition()) return DRAW_VALUE;

    Score ttScore = ttProbe(pos.getHash(), depth, alpha, beta);
    if (ttScore != UNKNOWN_SCORE) return ttScore;

    if (depth == 0) return quiescence(pos, alpha, beta, ply);

    Color color = pos.getSideToMove();

    MoveList moves = {pos, ply, false};

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

        if (shouldEnd()) return UNKNOWN_SCORE;

        if (score >= beta) {

            if (m.isQuiet()) {
                recordKillerMove(m, ply);
            }

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

// maxDepth necessary, because the way it's implemented it can find repetition cycles and it makes the StateStack overflow
// TODO smarter fix then limiting the pv line depth to a maximum of 10
std::string getPvLine(Position &pos) {
    Move m = getHashMove(pos.getHash());
    if (!pos.isRepetition() && m) {
        pos.makeMove(m);
        std::string str = m.str() + " " + getPvLine(pos);
        pos.undoMove(m);
        return str;
    } else {
        return "";
    }
}

Score searchRoot(Position &pos, Depth depth, bool uci) {

    clearKillerMoves();
    selectiveDepth = 0;

    Score score = search(pos, depth, -INF_SCORE, INF_SCORE, 0);

    if (score == UNKNOWN_SCORE) return UNKNOWN_SCORE;

    std::string pvLine = getPvLine(pos);
    if (uci) {
        Score absScore = std::abs(score);
        int mateDepth = MATE_VALUE - absScore;
        std::string scoreStr = "cp " + std::to_string(score);

        if (mateDepth <= 64) {
            int matePly;
            // We are giving the mate
            if (score > 0) {
                matePly = mateDepth / 2 + 1;

            } else {
                matePly = -(mateDepth / 2);
            }
            scoreStr = "mate " + std::to_string(matePly);
        }

        out("info", "depth", depth, "seldepth", selectiveDepth, "nodes", nodeCount, "score", scoreStr, "time",
            getSearchTime(), "nps", getNps(), "pv", pvLine);
    }


    return score;
}

void iterativeDeepening(Position pos, Depth depth, bool uci) {
    Move bestMove;

    for (Depth currDepth = 1; currDepth <= depth; currDepth++) {
        Score score = searchRoot(pos, currDepth, uci);
        if (score == UNKNOWN_SCORE) break;
        bestMove = getHashMove(pos.getHash());
    }

    globalAge++;

    if (uci)
        out("bestmove", bestMove);
}