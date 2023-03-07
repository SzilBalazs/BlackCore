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
#include "movegen.h"
#include "movelist.h"
#include "tt.h"
#include "tune.h"

SearchThread::SearchThread(const Position &pos, const SearchInfo &info) {

    // Store search info
    searchInfo = info;

    // Make a copy of the position
    position.loadFromPosition(pos);

    // Initialize time manager
    timeManager.init(info, pos.getSideToMove());

    // Refresh NNUE accumulator
    position.getState()->accumulator.refresh(position);
}

void SearchThread::start() {

    SearchStack stack[MAX_PLY + 1];
    for (int i = 0; i <= MAX_PLY; i++) {
        assert(i <= 127);
        stack[i].ply = i;
    }

    Move bestMove;
    Score prevScore = 0;
    for (Depth depth = 1; depth <= searchInfo.maxDepth; depth++) {

        Score score;
        Score alpha = -INF_SCORE;
        Score beta = INF_SCORE;

        if (depth >= ASPIRATION_DEPTH) {
            alpha = prevScore - ASPIRATION_DELTA;
            beta = prevScore + ASPIRATION_DELTA;
        }

        Score delta = ASPIRATION_DELTA;

        while (true) {

            score = search<ROOT_NODE>(stack, depth, alpha, beta);

            if (!timeManager.resourcesLeft()) {
                break;
            }

            if (score <= alpha) {
                beta = (alpha + beta) / 2;
                alpha = std::max(-ASPIRATION_BOUND, score - delta);
            } else if (score >= beta) {
                beta = std::min(ASPIRATION_BOUND, score + delta);
            } else {
                break;
            }

            delta += delta / 2;
        }

        if (!timeManager.resourcesLeft())
            break;

        if (searchInfo.uciMode)
            out("info", "depth", (int) depth, "score", "cp", score, "nodes", nodes, "pv", getPvLine());

        bestMove = pvArray[0][0];
        prevScore = score;
    }

    if (searchInfo.uciMode)
        out("bestmove", bestMove);
}

std::string SearchThread::getPvLine() {
    std::string pv;

    for (int i = 0; i < pvLength[0]; i++) {
        pv += pvArray[0][i].str() + " ";
    }

    return pv;
}

int64_t SearchThread::getNodes() const {
    return nodes;
}

int64_t SearchThread::getNps() const {
    return timeManager.calcNps(nodes);
}

template<NodeType nodeType>
Score SearchThread::qsearch(SearchStack *stack, Score alpha, Score beta) {
    constexpr bool pvNode = nodeType != NON_PV_NODE;
    constexpr bool nonPvNode = !pvNode;

    Move bestMove;

    selectivePly = std::max(selectivePly, stack->ply);

    if (stack->ply >= MAX_PLY) {
        return position.eval();
    }

    if ((nodes & 1023) == 0 && !timeManager.resourcesLeft()) {
        return UNKNOWN_SCORE;
    }

    Score bestScore = stack->eval = position.eval();

    if (bestScore >= beta) {
        return beta;
    }

    if (bestScore > alpha) {
        alpha = bestScore;
    }

    MoveList moves = MoveList<LIST_Q>(position, history, stack, MOVE_NULL);

    while (!moves.empty()) {
        Move move = moves.nextMove();

        nodes++;
        position.makeMove(move);

        Score score = -qsearch<nodeType>(stack + 1, -beta, -alpha);

        position.undoMove(move);

        if ((nodes & 1023) == 0 && !timeManager.resourcesLeft()) {
            return UNKNOWN_SCORE;
        }

        bestScore = std::max(bestScore, score);

        if (score >= beta) {

            return beta;
        }

        if (score > alpha) {
            alpha = score;
            bestMove = move;
        }
    }

    return bestScore;
}

template<NodeType nodeType>
Score SearchThread::search(SearchStack *stack, Depth depth, Score alpha, Score beta) {
    constexpr bool rootNode = nodeType == ROOT_NODE;
    constexpr bool pvNode = nodeType != NON_PV_NODE;
    constexpr bool nonRootNode = !rootNode;
    constexpr bool nonPvNode = !pvNode;
    constexpr NodeType nextNodeType = rootNode ? PV_NODE : nodeType;
    const Score matePly = MATE_VALUE - stack->ply;

    Score bestScore = -INF_SCORE;
    EntryFlag ttFlag = TT_ALPHA;
    Move bestMove;

    pvLength[stack->ply] = stack->ply;

    if (stack->ply >= MAX_PLY) {
        return position.eval();
    }

    if ((nodes & 1023) == 0 && !timeManager.resourcesLeft()) {
        return UNKNOWN_SCORE;
    }

    if (nonRootNode) {

        if (position.isRepetition() || position.getMove50() >= 99)
            return 1 - (nodes & 3);
    }

    /*
     * Transposition table probing
     *
     * Check the transposition table for information about this position. If the
     * node is a singular search root skip this step.
     */
    bool ttHit = false;
    TTEntry ttEntry = ttProbe(position.getHash(), stack->ply, ttHit);

    /*
     * TT cutoffs
     *
     * If this is a not a PV node and the transposition entry was saved by a
     * big enough depth search, return the evaluation from TT.
     */
    if (ttHit && nonPvNode && ttEntry.depth >= depth &&
        (ttEntry.flag == TT_EXACT || (ttEntry.flag == TT_ALPHA && ttEntry.eval <= alpha) || (ttEntry.flag == TT_BETA && ttEntry.eval >= beta))) {
        return ttEntry.eval;
    }

    if (depth <= 0) {
        return qsearch<nextNodeType>(stack, alpha, beta);
    }

    stack->eval = position.eval();
    bool inCheck = bool(getAttackers(position, position.pieces<KING>(position.getSideToMove()).lsb()));

    if (rootNode || inCheck)
        goto search_moves;

    if (nonPvNode && depth <= RFP_DEPTH && stack->eval - RFP_DEPTH_MULTI * depth >= beta && std::abs(beta) < TB_WORST_WIN)
        return beta;

    if (nonPvNode && depth >= NMP_DEPTH && stack->eval >= beta && !position.isZugzwang()) {
        Depth R = NMP_BASE + depth / NMP_DEPTH_MULTI;

        position.makeNullMove();
        Score score = -search<NON_PV_NODE>(stack + 1, depth - R, -beta, -beta + 1);
        position.undoNullMove();

        if (score >= beta) {
            if (std::abs(score) > TB_WORST_WIN)
                return beta;
            return score;
        }
    }

search_moves:
    MoveList moves = MoveList<LIST_AB>(position, history, stack, ttEntry.hashMove);

    if (moves.empty()) {
        return inCheck ? -matePly : 0;
    }

    Move quietMoves[200];
    int index = 0, quiets = 0;
    while (!moves.empty()) {
        Move move = moves.nextMove();

        Depth newDepth = depth - 1;
        nodes++;

        position.makeMove(move);

        Score score = UNKNOWN_SCORE;

        if (nonPvNode || index > 0) {
            score = -search<NON_PV_NODE>(stack + 1, newDepth, -alpha - 1, -alpha);
        }

        if (pvNode && (index == 0 || (alpha < score && score < beta))) {
            score = -search<PV_NODE>(stack + 1, newDepth, -beta, -alpha);
        }

        position.undoMove(move);

        if ((nodes & 1023) == 0 && !timeManager.resourcesLeft()) {
            return UNKNOWN_SCORE;
        }

        assert(score != UNKNOWN_SCORE);
        bestScore = std::max(bestScore, score);

        if (score >= beta) {

            history.updateHistory(position, quietMoves, quiets, move, stack->ply, std::min(1500, depth * 100));
            ttSave(position.getHash(), depth, beta, TT_BETA, move, stack->ply);

            return beta;
        }

        if (score > alpha) {
            alpha = score;
            bestMove = move;

            // Update PV-line
            pvArray[stack->ply][stack->ply] = move;
            for (int i = stack->ply + 1; i < pvLength[stack->ply + 1]; i++) {
                pvArray[stack->ply][i] = pvArray[stack->ply + 1][i];
            }
            pvLength[stack->ply] = pvLength[stack->ply + 1];
        }

        index++;
        quietMoves[quiets++] = move;
    }

    ttSave(position.getHash(), depth, bestScore, ttFlag, bestMove, stack->ply);
    return bestScore;
}