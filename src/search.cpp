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
#include "timeman.h"
#include "tt.h"
#include "uci.h"

#include <algorithm>
#include <cmath>

#ifdef TUNE

Score DELTA_MARGIN = 252;

Score RAZOR_MARGIN = 155;

Depth RFP_DEPTH = 8;
Score RFP_DEPTH_MULTIPLIER = 56;
Score RFP_IMPROVING_MULTIPLIER = 46;

Depth NULL_MOVE_DEPTH = 2;
Depth NULL_MOVE_BASE_R = 3;
Depth NULL_MOVE_R_SCALE = 3;

Depth LMR_DEPTH = 3;
double LMR_BASE = 1;
double LMR_SCALE = 1.65;
int LMR_INDEX = 2;

Depth LMP_DEPTH = 4;
int LMP_MOVES = 5;

Depth ASPIRATION_DEPTH = 9;
Score ASPIRATION_DELTA = 28;
Score ASPIRATION_BOUND = 3000;

Score SEE_MARGIN = 2;

#endif

Ply selectiveDepth = 0;
Move bestPV;

// Move index -> depth
Depth reductions[200][MAX_PLY + 1];

void initLmr() {
    for (int moveIndex = 0; moveIndex < 200; moveIndex++) {
        for (Depth depth = 0; depth < MAX_PLY; depth++) {

            reductions[moveIndex][depth] = std::max(2, Depth(LMR_BASE + (log((double) moveIndex) * log((double) depth) / LMR_SCALE)));
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
            (pieceAttacks<QUEEN>(square, occ) & pos.pieces<QUEEN>())) &
           occ;
}

Score see(const Position &pos, Move move) {
    Score e[32];
    Depth d = 0;
    Square from = move.getFrom();
    Square to = move.getTo();

    e[0] = move.equalFlag(EP_CAPTURE) ? PIECE_VALUES[PAWN] : PIECE_VALUES[pos.pieceAt(to).type];

    Bitboard rooks = pos.pieces<ROOK>() | pos.pieces<QUEEN>();
    Bitboard bishops = pos.pieces<BISHOP>() | pos.pieces<QUEEN>();
    Bitboard occ = pos.occupied() ^ Bitboard(to);
    Bitboard attacker = from;
    Bitboard attackers = getAllAttackers(pos, to, occ);

    Color stm = pos.pieceAt(to).color;
    PieceType type = pos.pieceAt(from).type;

    do {
        d++;
        e[d] = PIECE_VALUES[type] - e[d - 1];

        if (std::max(-e[d - 1], e[d]) < 0)
            break;

        occ ^= attacker;
        attackers ^= attacker;
        if (type == ROOK || type == QUEEN)
            attackers |= rookAttacks(to, occ) & rooks & occ;
        if (type == PAWN || type == BISHOP || type == QUEEN)
            attackers |= bishopAttacks(to, occ) & bishops & occ;
        attacker = leastValuablePiece(pos, attackers, stm, type);
        stm = EnemyColor(stm);

    } while (attacker);

    while (--d) {
        e[d - 1] = -std::max(-e[d - 1], e[d]);
    }

    return e[0];
}

template<NodeType type>
Score quiescence(Position &pos, Score alpha, Score beta, Ply ply) {

    constexpr bool pvNode = type != NON_PV_NODE;
    constexpr bool nonPvNode = !pvNode;

    if (shouldEnd())
        return UNKNOWN_SCORE;

    if (ply > selectiveDepth) {
        selectiveDepth = ply;
    }

    bool ttHit = false;
    TTEntry *ttEntry = ttProbe(pos.getHash(), ttHit, 0, alpha, beta);
    if (ttHit && nonPvNode && (ttEntry->flag == EXACT || (ttEntry->flag == ALPHA && ttEntry->eval <= alpha) || (ttEntry->flag == BETA && ttEntry->eval >= beta))) {
        return ttEntry->eval;
    }

    Score staticEval = eval(pos);

    if (ply >= MAX_PLY) {
        return staticEval;
    }

    if (staticEval >= beta) {
        return beta;
    }

    if (staticEval > alpha) {
        alpha = staticEval;
    }

    MoveList moves = {pos, ply, Move(), true, false};
    EntryFlag ttFlag = ALPHA;
    Move bestMove;

    while (!moves.empty()) {

        Move m = moves.nextMove();

        // Delta pruning
        if (m.isPromo() * PIECE_VALUES[QUEEN] + PIECE_VALUES[pos.pieceAt(m.getTo()).type] +
                    staticEval + DELTA_MARGIN <
            alpha)
            continue;

        // SEE pruning
        if (alpha > -WORST_MATE && see(pos, m) < -SEE_MARGIN)
            continue;

        pos.makeMove(m);

        Score score = -quiescence<type>(pos, -beta, -alpha, ply + 1);

        pos.undoMove(m);

        if (shouldEnd())
            return UNKNOWN_SCORE;

        if (score >= beta) {
            ttSave(pos.getHash(), 0, score, BETA, m);
            return beta;
        }

        if (score > alpha) {
            alpha = score;
            ttFlag = EXACT;
            bestMove = m;
        }
    }

    ttSave(pos.getHash(), 0, alpha, ttFlag, bestMove);
    return alpha;
}

template<NodeType type>
Score search(Position &pos, SearchStack *stack, Depth depth, Score alpha, Score beta, Ply ply) {

    constexpr bool rootNode = type == ROOT_NODE;
    constexpr bool pvNode = type != NON_PV_NODE;
    constexpr bool notRootNode = !rootNode;
    constexpr bool nonPvNode = !pvNode;
    constexpr NodeType nextPv = rootNode ? PV_NODE : type;
    const bool isSingularRoot = !stack->excludedMove.isNull();

    if (shouldEnd())
        return UNKNOWN_SCORE;

    if (notRootNode && pos.getMove50() >= 3 && pos.isRepetition()) {
        return DRAW_VALUE;
    }

    bool ttHit = false;
    Score matePly = MATE_VALUE - ply;
    TTEntry *ttEntry = isSingularRoot ? nullptr : ttProbe(pos.getHash(), ttHit, depth, alpha, beta);

    if (ttHit && nonPvNode &&
        ttEntry->depth >= depth && (ttEntry->flag == EXACT || (ttEntry->flag == ALPHA && ttEntry->eval <= alpha) || (ttEntry->flag == BETA && ttEntry->eval >= beta))) {
        return ttEntry->eval;
    }

    // Mate distance pruning
    if (notRootNode) {
        if (alpha < -matePly)
            alpha = -matePly;
        if (beta > matePly - 1)
            beta = matePly - 1;
        if (alpha >= beta)
            return alpha;
    }

    if (ply >= MAX_PLY) {
        return eval(pos);
    }

    if (depth <= 0)
        return quiescence<nextPv>(pos, alpha, beta, ply);

    Color color = pos.getSideToMove();
    bool inCheck = bool(getAttackers(pos, pos.pieces<KING>(color).lsb()));

    Score staticEval = stack->eval = eval(pos);

    bool improving = ply >= 2 && staticEval >= (stack - 2)->eval;

    if (notRootNode && !inCheck && !isSingularRoot) {

        // Razoring
        if (depth == 1 && nonPvNode && staticEval + RAZOR_MARGIN < alpha) {
            return quiescence<NON_PV_NODE>(pos, alpha, beta, ply);
        }

        // Reverse futility pruning
        if (depth <= RFP_DEPTH &&
            staticEval - RFP_DEPTH_MULTIPLIER * depth + RFP_IMPROVING_MULTIPLIER * improving >= beta &&
            std::abs(beta) < WORST_MATE)
            return beta;

        // Null move pruning
        if (nonPvNode && !(stack - 1)->move.isNull() && depth >= NULL_MOVE_DEPTH && staticEval >= beta) {
            // We don't want to make a null move in a Zugzwang position
            if (pos.pieces<KNIGHT>(color) | pos.pieces<BISHOP>(color) | pos.pieces<ROOK>(color) |
                pos.pieces<QUEEN>(color)) {

                Depth R = NULL_MOVE_BASE_R + depth / NULL_MOVE_R_SCALE;

                stack->move = Move();
                pos.makeNullMove();
                Score score = -search<NON_PV_NODE>(pos, stack + 1, depth - R, -beta, -beta + 1, ply + 1);
                pos.undoNullMove();

                if (score >= beta) {
                    if (std::abs(score) > WORST_MATE)
                        return beta;
                    return score;
                }
            }
        }

        // Internal iterative deepening
        if (!ttHit && pvNode)
            depth--;
        if (!ttHit && depth >= 5)
            depth--;

        if (depth <= 0)
            return quiescence<nextPv>(pos, alpha, beta, ply);
    }

    MoveList moves = {pos, ply, (ply >= 1 ? (stack - 1)->move : Move()), false, rootNode && depth >= 3};
    if (moves.count == 0) {
        if (isSingularRoot)
            return alpha;

        if (inCheck) {
            return -matePly;
        } else {
            return DRAW_VALUE;
        }
    }

    Move bestMove;
    EntryFlag ttFlag = ALPHA;
    int index = 0;
    std::vector<Move> quiets;
    while (!moves.empty()) {

        Move m = moves.nextMove();
        stack->move = m;

        if (m == stack->excludedMove) continue;

        U64 nodesBefore = nodeCount;

        if (rootNode) {
            if (getSearchTime() > 6000) out("info", "depth", depth, "currmove", m, "currmovenumber", index + 1);
        }

        Score score;

        // We can prune the move in some cases
        if (notRootNode && nonPvNode && !inCheck && alpha > -WORST_MATE) {

            if (depth <= FUTILITY_DEPTH && m.isQuiet() &&
                staticEval + FUTILITY_MARGIN + FUTILITY_MARGIN_DEPTH * depth + FUTILITY_MARGIN_IMPROVING * improving <
                        alpha)
                continue;

            // Late move/movecount pruning
            // This will also prune losing captures
            if (depth <= LMP_DEPTH && index >= LMP_MOVES + depth * depth && m.isQuiet())
                continue;
        }

        // Extensions
        Depth extensions = 0;

        if (inCheck) extensions = 1;
        else if (notRootNode && depth >= SINGULAR_DEPTH && ttHit && m == ttEntry->hashMove && !isSingularRoot && ttEntry->flag == BETA && ttEntry->depth >= depth - 3) {
            // This implementation is heavily inspired by StockFish & Alexandria
            Score singularBeta = ttEntry->eval - depth * 3;
            Depth singularDepth = (depth - 1) / 2;

            stack->excludedMove = m;
            score = search<NON_PV_NODE>(pos, stack, singularDepth, singularBeta - 1, singularBeta, ply);
            stack->excludedMove = Move();

            if (score < singularBeta) {
                extensions = 1;
            } else if (singularBeta >= beta) {
                return singularBeta;
            } else if (ttEntry->eval >= beta) {
                extensions = -1;
            }
        }

        pos.makeMove(m);

        ttPrefetch(pos.getHash());

        // Late move reduction
        if (!inCheck && depth >= LMR_DEPTH && index >= LMR_INDEX && !m.isPromo() &&
            m.isQuiet()) {

            Depth R = reductions[index][depth];

            R += improving;
            R -= pvNode;
            R -= (killerMoves[ply][0] == m || killerMoves[ply][1] == m) || (ply >= 1 && counterMoves[(stack - 1)->move.getFrom()][(stack - 1)->move.getTo()] == m);

            Depth newDepth = std::clamp(depth - R + extensions, 1, depth - 1);

            score = -search<NON_PV_NODE>(pos, stack + 1, newDepth,
                                         -alpha - 1, -alpha, ply + 1);

            if (score > alpha && R > 1) {
                score = -search<NON_PV_NODE>(pos, stack + 1, depth - 1 + extensions, -alpha - 1, -alpha, ply + 1);
            }

        } else if (nonPvNode || index != 0) {
            score = -search<NON_PV_NODE>(pos, stack + 1, depth - 1 + extensions, -alpha - 1, -alpha, ply + 1);
        }

        if (pvNode && (index == 0 || (score > alpha && score < beta))) {
            score = -search<nextPv>(pos, stack + 1, depth - 1 + extensions, -beta, -alpha, ply + 1);
        }

        pos.undoMove(m);

        if (rootNode) {
            recordNodesSearched(m, nodeCount - nodesBefore);
        }

        if (shouldEnd())
            return UNKNOWN_SCORE;

        if (score >= beta) {

            if (!isSingularRoot) {
                if (m.isQuiet()) {
                    recordKillerMove(m, ply);
                    if (ply >= 1 && !(stack - 1)->move.isNull()) recordCounterMove((stack - 1)->move, m);
                    recordHHMove(m, color, depth * 10);

                    for (Move move : quiets) {
                        recordHHMove(move, color, -depth * 10);
                    }
                }

                ttSave(pos.getHash(), depth, beta, BETA, m);
            }

            return beta;
        }

        if (score > alpha) {
            alpha = score;
            bestMove = m;
            ttFlag = EXACT;
        }

        if (m.isQuiet()) quiets.push_back(m);
        index++;
    }

    if (!isSingularRoot)
        ttSave(pos.getHash(), depth, alpha, ttFlag, bestMove);

    return alpha;
}

std::string getPvLine(Position &pos) {
    Move m = getHashMove(pos.getHash());
    if (!pos.isRepetition() && !m.isNull()) {
        pos.makeMove(m);
        std::string str = m.str() + " " + getPvLine(pos);
        pos.undoMove(m);
        return str;
    } else {
        return "";
    }
}

Score searchRoot(Position &pos, Score prevScore, Depth depth, bool uci) {

    globalAge++;
    clearTables();
    selectiveDepth = 0;
    SearchStack stateStack[MAX_PLY + 1];
    Score alpha = -INF_SCORE;
    Score beta = INF_SCORE;

    if (depth >= ASPIRATION_DEPTH) {
        alpha = prevScore - ASPIRATION_DELTA;
        beta = prevScore + ASPIRATION_DELTA;
    }

    int iter = 1;
    while (true) {
        if (shouldEnd())
            return UNKNOWN_SCORE;

        if (alpha < -ASPIRATION_BOUND)
            alpha = -INF_SCORE;
        if (beta > ASPIRATION_BOUND)
            beta = INF_SCORE;

        Score score = search<ROOT_NODE>(pos, stateStack + 1, depth, alpha, beta, 0);

        if (score == UNKNOWN_SCORE)
            return UNKNOWN_SCORE;

        if (score <= alpha) {
            alpha = std::max(alpha - iter * iter * ASPIRATION_DELTA, -INF_SCORE);
        } else if (score >= beta) {
            beta = std::min(beta + iter * iter * ASPIRATION_DELTA, INF_SCORE);
        } else {
            bestPV = getHashMove(pos.getHash());

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

        iter++;
    }
}

void iterativeDeepening(Position pos, Depth depth, bool uci) {

    pos.getState()->accumulator.refresh(pos);
    clearNodesSearchedTable();

    Score prevScore;
    Move bestMove;

    int stability = 0;

    for (Depth currDepth = 1; currDepth <= depth; currDepth++) {
        Score score = searchRoot(pos, prevScore, currDepth, uci);
        if (score == UNKNOWN_SCORE)
            break;

        // We only care about stability if we searched enough depth
        if (currDepth >= 16) {
            if (bestMove != bestPV) {
                stability -= 10;
            } else {
                if (std::abs(prevScore - score) >= std::max(prevScore / 10, 50)) {
                    stability -= 4;
                } else {
                    stability += 1;
                }
            }

            allocateTime(stability);
        }

        prevScore = score;
        bestMove = bestPV;
    }

    if (uci) {
        out("bestmove", bestMove);
    }

    searchStopped() = true;
}

#include <thread>

std::thread th;

void joinThread(bool waitToFinish) {
    if (!waitToFinish)
        stopSearch();

    if (th.joinable())
        th.join();
}

void startSearch(SearchInfo &searchInfo, Position &pos, int threadCount) {

    joinThread(false);

    Color stm = pos.getSideToMove();
    if (stm == WHITE) {
        initTimeMan(searchInfo.wtime, searchInfo.winc, searchInfo.movestogo, searchInfo.movetime, searchInfo.maxNodes);
    } else {
        initTimeMan(searchInfo.btime, searchInfo.binc, searchInfo.movestogo, searchInfo.movetime, searchInfo.maxNodes);
    }

    th = std::thread(iterativeDeepening, pos, searchInfo.maxDepth, searchInfo.uciMode);
}