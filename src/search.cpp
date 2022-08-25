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

#include <cmath>

Ply selectiveDepth = 0;

// Move index -> depth
Depth reductions[200][64];

void initLmr() {
    for (int moveIndex = 0; moveIndex < 200; moveIndex++) {
        for (Depth depth = 0; depth < 64; depth++) {
            reductions[moveIndex][depth] = moveIndex > 6 ? depth / 3 : 2;
        }
    }
}

Bitboard leastValuablePiece(const Position &pos, Bitboard attackers, Color stm, PieceType &type) {
    for (PieceType t : PIECE_TYPES_BY_VALUE) {
        Bitboard s = attackers & pos.pieces(stm, t);
        if (s) {
            type = t;
            return s & -s.bb;
        }
    }
    return 0;
}

Bitboard getAllAttackers(const Position &pos, Square square, Bitboard occ) {
    return (((pawnMask(square, WHITE) | pawnMask(square, BLACK)) & pos.pieces<PAWN>()) |
            (pieceAttacks<KNIGHT>(square, occ) & pos.pieces<KNIGHT>()) |
            (pieceAttacks<BISHOP>(square, occ) & pos.pieces<BISHOP>()) |
            (pieceAttacks<ROOK>(square, occ) & pos.pieces<ROOK>()) |
            (pieceAttacks<QUEEN>(square, occ) & pos.pieces<QUEEN>())) & occ;
}

Score see(const Position &pos, Move move) {
    Score e[32];
    Depth d = 0;
    Square from = move.getFrom();
    Square to = move.getTo();

    e[0] = PIECE_VALUES[pos.pieceAt(to).type].mg;

    Bitboard rooks = pos.pieces<ROOK>() | pos.pieces<QUEEN>();
    Bitboard bishops = pos.pieces<BISHOP>() | pos.pieces<QUEEN>();
    Bitboard occ = pos.occupied() ^ Bitboard(to);
    Bitboard attacker = from;
    Bitboard attackers = getAllAttackers(pos, to, occ);

    Color stm = pos.pieceAt(to).color;
    PieceType type = pos.pieceAt(from).type;

    do {
        d++;
        e[d] = PIECE_VALUES[type].mg - e[d-1];

        if (std::max(-e[d-1], e[d]) < 0) break;

        occ ^= attacker;
        attackers ^= attacker;
        if (type == ROOK || type == QUEEN)
            attackers |= rookAttacks(to, occ) & rooks & occ;
        if (type == PAWN || type == BISHOP || type == QUEEN)
            attackers |= bishopAttacks(to, occ) & bishops & occ;
        attacker = leastValuablePiece(pos, attackers, stm, type);
        stm = EnemyColor(stm);

    } while(attacker);

    while (--d) {
        e[d-1] = -std::max(-e[d-1], e[d]);
    }

    return e[0];
}



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

        if (alpha > -WORST_MATE && see(pos, m) < -SEE_PRUNING_MARGIN)
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

Score search(Position &pos, SearchState *state, Depth depth, Score alpha, Score beta, Ply ply) {

    if (shouldEnd()) return UNKNOWN_SCORE;

    if (pos.getMove50() >= 4 && ply > 0 && pos.isRepetition()) return DRAW_VALUE;

    bool ttHit = false;
    Score matePly = MATE_VALUE - ply;
    Score ttScore = ttProbe(pos.getHash(), ttHit, depth, alpha, beta);
    if (ttScore != UNKNOWN_SCORE) return ttScore;

    // Mate distance pruning
    if (ply > 0) {
        if (alpha < -matePly) alpha = -matePly;
        if (beta > matePly - 1) beta = matePly - 1;
        if (alpha >= beta) return alpha;
    }

    if (depth <= 0) return quiescence(pos, alpha, beta, ply);

    MoveList moves = {pos, ply, false};

    Color color = pos.getSideToMove();
    bool inCheck = bool(getAttackers(pos, pos.pieces<KING>(color).lsb()));

    if (moves.count == 0) {
        if (inCheck) {
            return -matePly;
        } else {
            return DRAW_VALUE;
        }
    }

    bool pvNode = beta - alpha > 1;

    Score staticEval = state->eval = eval(pos);

    if (ply > 0 && !inCheck) {

        bool improving = ply >= 2 && staticEval >= (state-2)->eval;

        // Razoring
        if (depth == 1 && !pvNode && staticEval + RAZOR_MARGIN < alpha) {
            return quiescence(pos, alpha, beta, ply);
        }

        // Reverse futility pruning
        if (depth <= RFP_DEPTH && staticEval - RFP_DEPTH_MULTIPLIER * (int) depth >= beta &&
            std::abs(beta) < WORST_MATE)
            return beta;

        // Null move pruning
        if (!pvNode && !(state - 1)->move.isNull() && depth >= NULL_MOVE_DEPTH && staticEval >= beta) {
            // We don't want to make a null move in a Zugzwang position
            if (pos.pieces<KNIGHT>(color) | pos.pieces<BISHOP>(color) | pos.pieces<ROOK>(color) |
                pos.pieces<QUEEN>(color)) {
                state->move = Move();
                pos.makeNullMove();
                Score score = -search(pos, state + 1, depth - NULL_MOVE_REDUCTION, -beta, -beta + 1, ply + 1);
                pos.undoNullMove();

                if (score >= beta) {
                    if (std::abs(score) > WORST_MATE) return beta;
                    return score;
                }
            }
        }

        // Internal iterative deepening
        if (!ttHit && depth >= IID_DEPTH) depth--;
    }

    if (inCheck)
        depth++;

    Move bestMove;
    EntryFlag ttFlag = ALPHA;
    int index = 0;

    while (!moves.empty()) {

        Move m = moves.nextMove();
        state->move = m;

        Score score;

        pos.makeMove(m);

        if (index == 0) {
            score = -search(pos, state + 1, depth - 1, -beta, -alpha, ply + 1);
        } else {
            // Late move reduction
            if (!inCheck && depth >= LMR_DEPTH && index >= LMR_MIN_I + pvNode * LMR_PVNODE_I && m.isQuiet() &&
                m != killerMoves[ply][0] && m != killerMoves[ply][1]) {

                score = -search(pos, state + 1, depth - reductions[index][depth], -alpha - 1, -alpha, ply + 1);
            } else score = alpha + 1;


            // Principal variation search
            if (score > alpha) {
                score = -search(pos, state + 1, depth - 1, -alpha - 1, -alpha, ply + 1);

                if (score > alpha) {
                    score = -search(pos, state + 1, depth - 1, -beta, -alpha, ply + 1);
                }
            }

        }

        pos.undoMove(m);

        if (shouldEnd()) return UNKNOWN_SCORE;

        if (score >= beta) {

            if (m.isQuiet()) {
                recordKillerMove(m, ply);
                recordHHMove(m, color, depth);
            }

            ttSave(pos.getHash(), depth, beta, BETA, m);
            return beta;
        }

        if (score > alpha) {
            alpha = score;
            bestMove = m;
            ttFlag = EXACT;
        }

        index++;
    }

    ttSave(pos.getHash(), depth, alpha, ttFlag, bestMove);

    return alpha;
}

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

    clearTables();
    selectiveDepth = 0;
    SearchState stateStack[400];

    Score score = search(pos, stateStack + 1, depth, -INF_SCORE, INF_SCORE, 0);

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