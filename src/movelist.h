#ifndef BLACKCORE_MOVELIST_H
#define BLACKCORE_MOVELIST_H

#include "eval.h"
#include "movegen.h"
#include "tt.h"

enum GenType { NORMAL,
               TACTICAL };
enum GenStage {
    STAGE_HASH,
    STAGE_GEN_INIT,
    STAGE_GEN_PROMOTIONS,
    STAGE_PLAY_PROMOTIONS,
    STAGE_GEN_CAPTURES,
    STAGE_PLAY_CAPTURES,
    STAGE_GEN_QUIETS,
    STAGE_PLAY_QUIETS,
    STAGE_FINISHED
};

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

template<GenType type>
class MoveList {
private:
    Move hashMove, moves[150], *endMove;
    Score scores[150];
    int leftMoveIndex = 0, rightMoveIndex = 0;

    const Position &pos;
    Ply ply;
    GenCache cache;
    GenStage stage = type == NORMAL ? STAGE_HASH : STAGE_GEN_INIT;

    inline Score scoreMove(Move move) {
        if (move.isPromo()) {
            if (move.isSpecial1() && move.isSpecial2()) {// Queen promo
                return 1;
            } else {
                return 0;
            }
        } else if (move.isCapture()) {
            return 200000 + see(pos, move);
        } else if (killerMoves[ply][0] == move) {
            return 100000;
        } else if (killerMoves[ply][1] == move) {
            return 90000;
        }
        return historyTable[pos.getSideToMove()][move.getFrom()][move.getTo()];
    }

    inline Move sortBestMove() {
        Score best = leftMoveIndex;
        for (int index = leftMoveIndex + 1; index < rightMoveIndex; index++) {
            if (scores[index] > scores[best]) {
                best = index;
            }
        }

        std::swap(moves[leftMoveIndex], moves[best]);
        std::swap(scores[leftMoveIndex], scores[best]);

        return moves[leftMoveIndex++];
    }

    template<GenGoal goal>
    inline void genMoves() {
        endMove = generateMoves(goal, pos, moves, cache);
        leftMoveIndex = 0;
        rightMoveIndex = endMove - moves;

        for (int index = leftMoveIndex; index < rightMoveIndex; index++) {
            scores[index] = scoreMove(moves[index]);
        }
    }

public:
    inline MoveList(const Position &position, Ply ply) : pos(position), ply(ply) {
        hashMove = getHashMove(pos.getHash());
    }

    inline bool isDone() { return stage == STAGE_FINISHED; }

    inline Move nextMove() {
        switch (stage) {
            case STAGE_HASH:
                stage = STAGE_GEN_INIT;
                if (!hashMove.isNull() && isPseudoLegal(pos, hashMove))
                    return hashMove;

            case STAGE_GEN_INIT:
                stage = STAGE_GEN_PROMOTIONS;
                generateMoves(INIT, pos, moves, cache);

            case STAGE_GEN_PROMOTIONS:
                stage = STAGE_PLAY_PROMOTIONS;
                genMoves<PROMOTIONS>();

            case STAGE_PLAY_PROMOTIONS:
                if (leftMoveIndex < rightMoveIndex) {
                    Move move = sortBestMove();
                    return move == hashMove ? nextMove() : move;
                }

                stage = STAGE_GEN_CAPTURES;

            case STAGE_GEN_CAPTURES:
                stage = STAGE_PLAY_CAPTURES;
                genMoves<CAPTURES>();

            case STAGE_PLAY_CAPTURES:
                if (leftMoveIndex < rightMoveIndex) {
                    Move move = sortBestMove();
                    return move == hashMove ? nextMove() : move;
                }

                if constexpr (type == TACTICAL) {
                    stage = STAGE_FINISHED;
                    break;
                }

                stage = STAGE_GEN_QUIETS;

            case STAGE_GEN_QUIETS:
                stage = STAGE_PLAY_QUIETS;
                genMoves<QUIETS>();

            case STAGE_PLAY_QUIETS:
                if (leftMoveIndex < rightMoveIndex) {
                    Move move = sortBestMove();
                    return move == hashMove ? nextMove() : move;
                }

                stage = STAGE_FINISHED;

            case STAGE_FINISHED:
                return {};
        }

        return {};
    }
};

#endif// BLACKCORE_MOVELIST_H
