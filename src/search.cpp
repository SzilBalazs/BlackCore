// BlackCore is a chess engine
// Copyright (c) 2022-2023 SzilBalazs
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

#include "search.h"
#include "egtb.h"
#include "eval.h"
#include "threads.h"
#include "timeman.h"
#include "tt.h"
#include "uci.h"

#include <algorithm>
#include <cmath>
#include <thread>

// Move index -> depth
Depth reductions[200][MAX_PLY + 1];

std::mutex mNodesSearched;
U64 nodesSearched[64][64];

std::vector<ThreadData> tds;
std::vector<std::thread> ths;

// Sums up the node count of individual threads
U64 getTotalNodes() {
    U64 totalNodes = 0;
    for (ThreadData &td : tds) {
        totalNodes += td.nodes;
    }
    return totalNodes;
}

// Sumps up the TB hits of individual threads
U64 getTotalTBHits() {
    U64 totalHits = 0;
    for (ThreadData &td : tds) {
        totalHits += td.tbHits;
    }
    return totalHits;
}

// Initialize a lookup table for LMR reduction values
void initLmr() {
    for (int moveIndex = 0; moveIndex < 200; moveIndex++) {
        for (Depth depth = 0; depth < MAX_PLY; depth++) {

            reductions[moveIndex][depth] = Depth(std::max(1, int(LMR_BASE + (log((double) moveIndex) * log((double) depth) / LMR_SCALE))));
        }
    }
}

/*
 * Static-Exchange-Evaluation
 *
 * Returns the material change, after playing out every resulting capture of the move.
 */
bool see(const Position &pos, Move move, Score threshold) {
    assert(move.isCapture()); // Make sure move is a capture

    Square from = move.getFrom();
    Square to = move.getTo();

    // TODO EP captures
    if (move.isPromo()) return true;

    Score value = PIECE_VALUES[pos.pieceAt(to).type] - threshold;

    if (value < 0) return false;

    value -= PIECE_VALUES[pos.pieceAt(from).type];

    if (value >= 0) return true;

    Bitboard rooks = pos.pieces<ROOK>() | pos.pieces<QUEEN>();
    Bitboard bishops = pos.pieces<BISHOP>() | pos.pieces<QUEEN>();
    Bitboard occ = pos.occupied() ^ Bitboard(from) ^ Bitboard(to);

    // Initialize the current attacker as the piece that made the capture
    Bitboard attacker = from;
    // Get all attackers to the destination square
    Bitboard attackers = pos.getAllAttackers(to, occ);

    Color stm = EnemyColor(pos.pieceAt(from).color);

    while (true) {
        attackers &= occ;

        PieceType type;
        attacker = pos.leastValuablePiece(attackers, stm, type);

        if (!attacker)
            break;

        value = -value - 1 - PIECE_VALUES[type];
        stm = EnemyColor(stm);

        if (value >= 0) {
            if (type == KING && (attackers & pos.friendly(stm))) {
                stm = EnemyColor(stm);
            }
            break;
        }


        occ ^= attacker;

        if (type == ROOK || type == QUEEN)
            attackers |= rookAttacks(to, occ) & rooks & occ;
        if (type == PAWN || type == BISHOP || type == QUEEN)
            attackers |= bishopAttacks(to, occ) & bishops & occ;
    }

    return stm != pos.pieceAt(from).color;
}

/*
 * Quiescence search
 *
 * A special type of alpha-beta search, which only searches
 * captures, to improve the tactical stability of the main
 * search.
 */
template<NodeType type>
Score quiescence(Position &pos, ThreadData &td, SearchStack *stack, Score alpha, Score beta) {

    constexpr bool pvNode = type != NON_PV_NODE;
    constexpr bool nonPvNode = !pvNode;

    // Check if search should stop by asking the time manager
    if (shouldEnd(td.nodes, getTotalNodes()))
        return UNKNOWN_SCORE;

    // Update the maximum depth reached.
    td.selectiveDepth = std::max(td.selectiveDepth, stack->ply);

    /*
     * Transposition table probing
     *
     * Check if position has been searched before by probing
     * the transposition table
     */
    bool ttHit = false;
    TTEntry ttEntry = ttProbe(pos.getHash(), ttHit);

    /*
     * TT cutoffs
     *
     * If the position has been searched before,
     * return the score from the transposition table.
     */
    if (ttHit && nonPvNode && (ttEntry.flag == TT_EXACT || (ttEntry.flag == TT_ALPHA && ttEntry.eval <= alpha) || (ttEntry.flag == TT_BETA && ttEntry.eval >= beta))) {
        return ttEntry.eval;
    }

    // Get the evaluation of the position, which will be used as the stand pat score
    Score bestScore = eval(pos);

    // Return the evaluation if maximum ply is reached
    if (stack->ply >= MAX_PLY) {
        return bestScore;
    }

    /*
     * Standing pat
     *
     * As only tactical moves will be evaluated, static evaluation can be used to get a
     * lower-bound of the position score.
     */

    if (bestScore >= beta) {
        return beta;
    }

    if (bestScore > alpha) {
        alpha = bestScore;
    }

    // Generate all legal capture
    auto moves = MoveList<true, false>(pos, td, Move(), stack->ply);

    EntryFlag ttFlag = TT_ALPHA;
    Move bestMove;

    // Iterate through the generated moves
    while (!moves.empty()) {

        Move move = moves.nextMove();

        /*
         * Delta pruning
         *
         * If the static evaluation and the expected gain of this move plus a large margin is still
         * less than alpha the move can be safely skipped.
         */
        if (move.isPromo() * PIECE_VALUES[QUEEN] + PIECE_VALUES[pos.pieceAt(move.getTo()).type] +
                    bestScore + DELTA_MARGIN <
            alpha)
            continue;

        /*
         * Static-Exchange-Evaluation pruning
         *
         * If the move loses material we skip its evaluation
         */
        if (bestScore > TB_BEST_LOSS && !see(pos, move, 0))
            continue;

        td.nodes++; // Update total number of nodes searched

        pos.makeMove(move);

        Score score = -quiescence<type>(pos, td, stack + 1, -beta, -alpha);

        pos.undoMove(move);

        // Check if search should stop by asking the time manager
        if (shouldEnd(td.nodes, getTotalNodes()))
            return UNKNOWN_SCORE;

        bestScore = std::max(bestScore, score);

        // If the score is too good to be acceptable by our opponent return beta
        if (score >= beta) {
            // If beta cutoff happens save the information to the transposition table
            ttSave(pos.getHash(), 0, score, TT_BETA, move);

            return beta;
        }

        // If the score is better than alpha update alpha
        if (score > alpha) {
            alpha = score;
            ttFlag = TT_EXACT;
            bestMove = move;
        }
    }

    // Save information to the transposition table
    ttSave(pos.getHash(), 0, bestScore, ttFlag, bestMove);
    return bestScore;
}

template<NodeType type>
Score search(Position &pos, ThreadData &td, SearchStack *stack, Depth depth, Score alpha, Score beta) {

    constexpr bool rootNode = type == ROOT_NODE;
    constexpr bool pvNode = type != NON_PV_NODE;
    constexpr bool notRootNode = !rootNode;
    constexpr bool nonPvNode = !pvNode;
    constexpr NodeType nextPv = rootNode ? PV_NODE : type;
    const bool isSingularRoot = stack->excludedMove.isOk();
    const Move prevMove = (stack - 1)->move;
    const Score matePly = MATE_VALUE - stack->ply;

    Score bestScore = -INF_SCORE;
    Score maxScore = INF_SCORE;

    td.pvLength[stack->ply] = stack->ply;
    td.killerMoves[stack->ply + 1][0] = Move();
    td.killerMoves[stack->ply + 1][1] = Move();

    // Check if search should stop by asking the time manager
    if (shouldEnd(td.nodes, getTotalNodes()))
        return UNKNOWN_SCORE;

    if (notRootNode) {

        // If a repetition or fifty move rule happens return DRAW_VALUE.
        if (pos.isRepetition() || pos.getMove50() >= 99)
            return 1 - (td.nodes & 3);


        /*
         * Mate distance pruning
         *
         * If the position is "solved" - the shortest mate was found - update alpha and beta.
         */
        alpha = std::max(alpha, -matePly);
        beta = std::min(beta, matePly - 1);
        if (alpha >= beta)
            return alpha;
    }

    /*
     * Transposition table probing
     *
     * Check the transposition table for information about this position. If the
     * node is a singular search root skip this step.
     */
    bool ttHit = false;
    TTEntry ttEntry = isSingularRoot ? TTEntry() : ttProbe(pos.getHash(), ttHit);

    /*
     * TT cutoffs
     *
     * If this is a not a PV node and the transposition entry was saved by a
     * big enough depth search, return the evaluation from TT.
     */
    if (ttHit && nonPvNode && ttEntry.depth >= depth && prevMove.isOk() && pos.getMove50() < 90 &&
        (ttEntry.flag == TT_EXACT || (ttEntry.flag == TT_ALPHA && ttEntry.eval <= alpha) || (ttEntry.flag == TT_BETA && ttEntry.eval >= beta))) {
        return ttEntry.eval;
    }

    if (stack->ply >= MAX_PLY) {
        return eval(pos);
    }

    /*
     * Endgame tablebase probing
     *
     * https://www.chessprogramming.org/Syzygy_Bases
     */
    if (notRootNode) {
        unsigned int result = TBProbe(pos);
        if (result != TB_RESULT_FAILED) {
            EntryFlag flag = TT_EXACT;
            Score score = DRAW_VALUE;
            td.tbHits++;

            if (result == TB_WIN) {
                flag = TT_BETA;
                score = TB_WIN_SCORE - stack->ply;
            } else if (result == TB_LOSS) {
                flag = TT_ALPHA;
                score = TB_LOSS_SCORE + stack->ply;
            }

            if (flag == TT_EXACT || (flag == TT_ALPHA && score <= alpha) || (flag == TT_BETA && score >= beta)) {
                ttSave(pos.getHash(), depth, score, flag, Move());
                return score;
            }

            if (pvNode) {
                if (flag == TT_BETA) {
                    alpha = std::max(alpha, score);
                } else {
                    maxScore = score;
                }
            }
        }
    }

    // At depth 0 drop into quiescence search.
    if (depth <= 0)
        return quiescence<nextPv>(pos, td, stack, alpha, beta);

    Color color = pos.getSideToMove();
    bool inCheck = bool(getAttackers(pos, pos.pieces<KING>(color).lsb()));

    Score staticEval = stack->eval = eval(pos);

    // Improving boolean, first introduced by StockFish
    // If the position got better than 2 ply before, we can
    // except that it will further improve.
    bool improving = stack->ply >= 2 && staticEval >= (stack - 2)->eval;

    if (notRootNode && !inCheck) {

        // Internal iterative deepening to prevent search explosions.
        if (!ttHit && depth >= 5)
            depth--;

        /*
         * Razoring
         *
         * At depth 1 safely drop into quiescence search, if the static evaluation is very low.
         */
        if (depth == 1 && staticEval + RAZOR_MARGIN < alpha) {
            return quiescence<NON_PV_NODE>(pos, td, stack, alpha, beta);
        }

        /*
         * Reverse futility pruning
         *
         * At low depths if the static evaluation is very high return beta.
         */
        if (nonPvNode && depth <= RFP_DEPTH &&
            staticEval - RFP_DEPTH_MULTIPLIER * depth + RFP_IMPROVING_MULTIPLIER * improving >= beta &&
            std::abs(beta) < TB_WORST_WIN)
            return beta;

        /*
         * Null move pruning
         *
         * If the static evaluation is better than beta, give the turn to the opponent and hope that it
         * will stay big enough to cause a beta-cutoff in a reduced depth search.
         */
        if (nonPvNode && prevMove.isOk() && depth >= NULL_MOVE_DEPTH && staticEval >= beta && !isSingularRoot) {

            // Don't want to make a null move in a Zugzwang position
            if (pos.pieces<KNIGHT>(color) | pos.pieces<BISHOP>(color) | pos.pieces<ROOK>(color) |
                pos.pieces<QUEEN>(color)) {

                Depth R = NULL_MOVE_BASE_R + depth / NULL_MOVE_R_SCALE;

                stack->move = Move();
                pos.makeNullMove();
                Score score = -search<NON_PV_NODE>(pos, td, stack + 1, depth - R, -beta, -beta + 1);
                pos.undoNullMove();

                // If the score is still higher than beta, safely return score.
                if (score >= beta) {
                    if (std::abs(score) > TB_WORST_WIN)
                        return beta;
                    return score;
                }
            }
        }
    }

    auto moves = MoveList<false, rootNode>(pos, td, prevMove, stack->ply);

    // If there is no legal moves the position is either a checkmate or a stalemate.
    if (moves.count == 0) {
        if (isSingularRoot)
            return alpha;

        return inCheck ? -matePly : DRAW_VALUE;
    }

    Move bestMove;
    EntryFlag ttFlag = TT_ALPHA;
    int index = 0;
    std::vector<Move> quiets;
    while (!moves.empty()) {

        Move move = moves.nextMove(); // Currently searched move

        if (move == stack->excludedMove) continue;

        U64 nodesBefore = td.nodes;

        if (rootNode && td.uciMode && getSearchTime() > 2000) {
            printCurrMove(depth, index, move);
        }

        Score score;
        Score history = td.historyTable[color][move.getFrom()][move.getTo()];

        // Prune quiet moves if ...
        if (notRootNode && nonPvNode && !inCheck && bestScore > TB_BEST_LOSS && move.isQuiet() && !move.isPromo()) {

            // Futility pruning
            // ... the static evaluation is far below alpha.
            if (depth <= FUTILITY_DEPTH &&
                staticEval + FUTILITY_MARGIN + FUTILITY_MARGIN_DEPTH * depth + FUTILITY_MARGIN_IMPROVING * improving < alpha)
                continue;

            // Late move pruning
            // ... many moves had been made before.
            if (depth <= LMP_DEPTH && index >= LMP_MOVES + depth * depth)
                continue;
        }

        // Extensions
        Depth extensions = 0;

        /*
         * Check extension
         *
         * If the position is a check extend by 1 ply.
         */
        if (inCheck) extensions = 1;

        /*
         * Singular extension
         *
         * If 1 move is a lot better than all the others extend by 1 ply.
         * This implementation is heavily inspired by StockFish & Alexandria
         */
        else if (notRootNode && depth >= SINGULAR_DEPTH && ttHit && move == ttEntry.hashMove && !isSingularRoot && ttEntry.flag == TT_BETA &&
                 ttEntry.depth >= depth - 3 && std::abs(ttEntry.eval) < TB_WORST_WIN) {
            Score singularBeta = ttEntry.eval - depth * 3;
            Depth singularDepth = (depth - 1) / 2;

            stack->excludedMove = move;
            score = search<NON_PV_NODE>(pos, td, stack, singularDepth, singularBeta - 1, singularBeta);
            stack->excludedMove = Move();

            if (score < singularBeta) {
                extensions = 1;
            } else if (singularBeta >= beta) {
                return singularBeta;
            } else if (ttEntry.eval >= beta) {
                extensions = -1;
            }
        }

        stack->move = move;

        Depth newDepth = depth - 1 + extensions;

        td.nodes++;
        pos.makeMove(move);

        ttPrefetch(pos.getHash());

        /*
         * Late move reduction
         *
         * Reduce (or in rare cases) extend quiet moves later ranked by our move ordering.
         * https://www.chessprogramming.org/Late_Move_Reductions
         */
        if (!inCheck && depth >= LMR_DEPTH && index >= LMR_INDEX && !move.isPromo() &&
            move.isQuiet()) {

            Depth R = reductions[index][depth];

            R += !improving;
            R -= pvNode;
            R -= std::clamp(history / 3000, -1, 1);
            R -= td.killerMoves[stack->ply][0] == move || td.killerMoves[stack->ply][1] == move || td.counterMoves[prevMove.getFrom()][prevMove.getTo()] == move;

            Depth D = std::clamp(newDepth - R, 1, newDepth + 1);

            score = -search<NON_PV_NODE>(pos, td, stack + 1, D,
                                         -alpha - 1, -alpha);

            if (score > alpha && R > 0) {
                score = -search<NON_PV_NODE>(pos, td, stack + 1, newDepth, -alpha - 1, -alpha);
            }

        } else if (nonPvNode || index != 0) {
            score = -search<NON_PV_NODE>(pos, td, stack + 1, newDepth, -alpha - 1, -alpha);
        }

        if (pvNode && (index == 0 || (score > alpha && score < beta))) {
            score = -search<nextPv>(pos, td, stack + 1, newDepth, -beta, -alpha);
        }

        pos.undoMove(move);

        if (rootNode) {
            td.updateNodesSearched(move, td.nodes - nodesBefore);
        }

        // Check if search should stop by asking the time manager
        if (shouldEnd(td.nodes, getTotalNodes()))
            return UNKNOWN_SCORE;

        bestScore = std::max(bestScore, score);

        if (score >= beta) {

            if (!isSingularRoot) {
                if (move.isQuiet()) {

                    // Update history heuristics
                    td.updateKillerMoves(move, stack->ply);
                    if (prevMove.isOk())
                        td.updateCounterMoves(prevMove, move);
                    td.updateHH(move, color, depth * depth);

                    for (Move m : quiets) {
                        td.updateHH(m, color, -depth * depth);
                    }
                }

                // Save the information gathered into the transposition table.
                ttSave(pos.getHash(), depth, beta, TT_BETA, move);
            }
            return beta;
        }

        if (score > alpha) {
            alpha = score;
            bestMove = move;
            ttFlag = TT_EXACT;

            // Update PV-line
            td.pvArray[stack->ply][stack->ply] = move;
            for (int i = stack->ply + 1; i < td.pvLength[stack->ply + 1]; i++) {
                td.pvArray[stack->ply][i] = td.pvArray[stack->ply + 1][i];
            }
            td.pvLength[stack->ply] = td.pvLength[stack->ply + 1];
        }

        if (move.isQuiet())
            quiets.push_back(move);
        index++;
    }

    bestScore = std::min(bestScore, maxScore);

    // Only save the information gathered into the transposition table, if the node isn't a singular search root.
    if (!isSingularRoot)
        ttSave(pos.getHash(), depth, bestScore, ttFlag, bestMove);

    return bestScore;
}

// Returns the PV-line of the selected thread
std::string getPvLine(ThreadData &td) {
    std::string pv;

    for (int i = 0; i < td.pvLength[0]; i++) {
        pv += td.pvArray[0][i].str() + " ";
    }

    return pv;
}

/*
 * Aspiration window
 *
 * Expect that at a new depth the returned score by the main search will
 * stay close to the previous iteration of iterative deepening.
 * https://www.chessprogramming.org/Aspiration_Windows
 */
Score searchRoot(Position &pos, ThreadData &td, Score prevScore, Depth depth) {

    td.clear();

    SearchStack stateStack[MAX_PLY + 10];

    for (Ply i = 0; i <= MAX_PLY; i++) {
        stateStack[i].excludedMove = Move();
        stateStack[i].move = Move();
        stateStack[i].eval = UNKNOWN_SCORE;
        stateStack[i].ply = i - 4;
    }

    // Start at -inf and +inf bounds
    Score alpha = -INF_SCORE;
    Score beta = INF_SCORE;

    // If ASPIRATION_DEPTH is reached, assume that the previous iteration
    // gave us a close enough score.
    if (depth >= ASPIRATION_DEPTH && std::abs(prevScore) < TB_WORST_WIN) {
        alpha = prevScore - ASPIRATION_DELTA;
        beta = prevScore + ASPIRATION_DELTA;
    }

    int iter = 1;
    while (true) {
        // Check if search should stop by asking the time manager
        if (shouldEnd(td.nodes, getTotalNodes()))
            return UNKNOWN_SCORE;

        if (alpha < -ASPIRATION_BOUND)
            alpha = -INF_SCORE;
        if (beta > ASPIRATION_BOUND)
            beta = INF_SCORE;

        Score score = search<ROOT_NODE>(pos, td, stateStack + 4, depth, alpha, beta);

        if (score == UNKNOWN_SCORE)
            return UNKNOWN_SCORE;

        if (score <= alpha) {
            alpha = std::max(alpha - iter * iter * ASPIRATION_DELTA, -INF_SCORE);
        } else if (score >= beta) {
            beta = std::min(beta + iter * iter * ASPIRATION_DELTA, INF_SCORE);
        } else {

            std::string pvLine = getPvLine(td);
            if (td.uciMode) {
                Score absScore = std::abs(score);
                int mateDepth = MATE_VALUE - absScore;
                std::string scoreStr = "cp " + std::to_string(score);

                // Calculate mate depth if a mate was found.
                if (mateDepth <= 64) {
                    int matePly = score > 0 ? mateDepth / 2 + 1 : -(mateDepth / 2);
                    scoreStr = "mate " + std::to_string(matePly);
                }

                // Output information to the GUI
                printNewDepth(depth, td.selectiveDepth, getTotalNodes(), getTTFull(), getTotalTBHits(), score, scoreStr, getSearchTime(), getNps(getTotalNodes()), pvLine);
            }

            return score;
        }

        iter++;
    }
}

/*
 * Iterative Deepening
 *
 * https://www.chessprogramming.org/Iterative_Deepening
 */
void iterativeDeepening(int id, Depth depth) {

    ThreadData &td = tds[id];
    Position &pos = tds[id].position;

    td.reset();
    pos.getState()->accumulator.refresh(pos);

    Score prevScore = 0;
    Move bestMove;

    int bmStability = 0;

    for (Depth currDepth = 1; currDepth <= depth; currDepth++) {
        Score score = searchRoot(pos, td, prevScore, currDepth + (td.threadId & 1));
        if (score == UNKNOWN_SCORE)
            break;

        // Only care about time management if we searched enough depth, and we are the main thread.
        if (td.threadId == 0) {

            if (bestMove != td.pvArray[0][0]) {
                bmStability = 0;
            } else {
                bmStability++;
            }

            double factor = std::max(0.5, 1.1 - 0.03 * bmStability);

            if (score - prevScore > ASPIRATION_DELTA)
                factor *= 1.1;

            U64 bestMoveEffort = nodesSearched[td.pvArray[0][0].getFrom()][td.pvArray[0][0].getTo()];
            double notBestMove = 1.0 - double(bestMoveEffort) / double(getTotalNodes());
            factor *= std::max(0.5, 2 * notBestMove + 0.4);

            if (manageTime(factor) && currDepth >= 10) break;
        }

        prevScore = score;
        bestMove = td.pvArray[0][0];
    }

    if (td.uciMode) {
        out("bestmove", bestMove);
    }

    stopped = true;
}

// Join all the threads to the main thread.
void joinThreads(bool waitToFinish) {
    if (!waitToFinish)
        stopped = true;

    for (std::thread &th : ths) {
        if (th.joinable())
            th.join();
    }

    ths.clear();
    tds.clear();
}

// Starts the process of finding the best move.
void startSearch(SearchInfo &searchInfo, Position &pos, int threadCount) {
#ifdef DATA_FILTER
    out("Binary only for DATA_FILTER!");
    return;
#endif
    joinThreads(false);

    // Initializes ThreadData object for storing variables of threads.
    for (int idx = 0; idx < threadCount; idx++) {
        ThreadData td;
        td.threadId = idx;
        td.uciMode = searchInfo.uciMode && idx == 0;
        tds.emplace_back(td);
    }

    // Create a copy of the searched position, one for each thread.
    for (int idx = 0; idx < threadCount; idx++) {
        tds[idx].position.loadFromPosition(pos);
    }

    // Initializes time manager.
    Color stm = pos.getSideToMove();
    if (stm == WHITE) {
        initTimeMan(searchInfo.wtime, searchInfo.winc, searchInfo.movestogo, searchInfo.movetime, searchInfo.maxNodes);
    } else {
        initTimeMan(searchInfo.btime, searchInfo.binc, searchInfo.movestogo, searchInfo.movetime, searchInfo.maxNodes);
    }

    if (!isInfiniteSearch() && searchInfo.uciMode) {
        bool tbHit = TBProbeRoot(pos);
        if (tbHit)
            return;
    }

    // Starts every thread.
    for (int idx = 0; idx < threadCount; idx++) {
        ths.emplace_back(iterativeDeepening, idx, searchInfo.maxDepth);
    }
}