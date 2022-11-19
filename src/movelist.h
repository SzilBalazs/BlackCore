#ifndef BLACKCORE_MOVELIST_H
#define BLACKCORE_MOVELIST_H

#include "movegen.h"
#include "tt.h"

enum GenType { NORMAL, TACTICAL };
enum GenStage {
  STAGE_HASH,
  STAGE_GEN_INIT,
  STAGE_GEN_PROMOTIONS,
  STAGE_PLAY_PROMOTIONS,
  STAGE_GEN_CAPTURES,
  STAGE_PLAY_CAPTURES,
  STAGE_PLAY_KILLER,
  STAGE_GEN_QUIETS,
  STAGE_PLAY_QUIETS
};

template <GenType type> class MoveList {
private:
  Move hashMove, moves[150], *endMove;
  Score scores[150];
  int leftMoveIndex = 0, rightMoveIndex = 0;

  Position &pos;
  GenCache cache;
  GenStage stage = STAGE_HASH;

  inline Score scorePromotion(Move move) {

    if (m == getHashMove(pos.getHash())) {
      return 1000000;
    } else if (m.isPromo()) {
      if (m.isSpecial1() && m.isSpecial2()) { // Queen promo
        return 900000;
      } else { // Anything else, under promotions should only be played in
               // really few cases
        return -100000;
      }
    } else if (m.isCapture()) {
      Square from = m.getFrom();
      Square to = m.getTo();

      if (see(pos, m) >= 0)
        return winningCapture[pos.pieceAt(from).type][pos.pieceAt(to).type];
      else
        return losingCapture[pos.pieceAt(from).type][pos.pieceAt(to).type];
    } else if (killerMoves[ply][0] == m) {
      return 750000;
    } else if (killerMoves[ply][1] == m) {
      return 700000;
    }
    return historyTable[pos.getSideToMove()][m.getFrom()][m.getTo()];
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

  template <GenGoal goal> inline void genMoves() {
    endMove = generateMoves(goal, pos, moves, cache);
    leftMoveIndex = 0;
    rightMoveIndex = endMove - moves;

    for (int index = leftMoveIndex; index < rightMoveIndex; index++) {
      scores[index] =
          hashMove == moves[index] ? -100000 : scoreMove(moves[index]);
    }
  }

public:
  inline MoveList(const Position &position) {
    pos = position;
    hashMove = getHashMove(pos.getHash());
  }

  inline Move nextMove() {
    switch (stage) {
    case STAGE_HASH:
      stage = STAGE_GEN_INIT;
      if (isPseudoLegal(pos, hashMove))
        return moves[0];

    case STAGE_GEN_INIT:
      stage = STAGE_GEN_PROMOTIONS;
      generateMoves(INIT, pos, moves, cache);

    case STAGE_GEN_PROMOTIONS:
      stage = STAGE_PLAY_PROMOTIONS;
      genMoves<PROMOTIONS>();

    case STAGE_PLAY_PROMOTIONS:
      if (leftMoveIndex != rightMoveIndex)
        return sortBestMove();

      stage = STAGE_GEN_CAPTURES;

    case STAGE_GEN_CAPTURES:
      stage = STAGE_PLAY_CAPTURES;
      genMoves<CAPTURES>();
    }
  }
};

#endif // BLACKCORE_MOVELIST_H
