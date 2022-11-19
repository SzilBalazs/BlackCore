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

#include "movegen.h"

inline Move *makePromo(Move *moves, Square from, Square to) {
  *moves++ = Move(from, to, PROMO_KNIGHT);
  *moves++ = Move(from, to, PROMO_BISHOP);
  *moves++ = Move(from, to, PROMO_ROOK);
  *moves++ = Move(from, to, PROMO_QUEEN);
  return moves;
}

inline Move *makePromoCapture(Move *moves, Square from, Square to) {
  *moves++ = Move(from, to, PROMO_CAPTURE_KNIGHT);
  *moves++ = Move(from, to, PROMO_CAPTURE_BISHOP);
  *moves++ = Move(from, to, PROMO_CAPTURE_ROOK);
  *moves++ = Move(from, to, PROMO_CAPTURE_QUEEN);
  return moves;
}

bool isPseudoLegal(const Position &pos, Move move) {
  Square from = move.getFrom();
  Square to = move.getTo();
  Piece piece = pos.pieceAt(from);

  Bitboard attacks = pieceAttacks(piece.type, from, pos.occupied()) &
                     pos.enemyOrEmpty(piece.color);

  return attacks.get(to);
}

template <Color color>
inline Bitboard getAttackedSquares(const Position &pos, Bitboard occupied) {

  constexpr Direction UP_LEFT = color == WHITE ? NORTH_WEST : -NORTH_WEST;
  constexpr Direction UP_RIGHT = color == WHITE ? NORTH_EAST : -NORTH_EAST;

  Bitboard pawns = pos.pieces<color, PAWN>();
  Bitboard pieces = pos.friendly<color>() & ~pawns;
  Bitboard result = step<UP_LEFT>(pawns) | step<UP_RIGHT>(pawns);

  while (pieces) {
    Square from = pieces.popLsb();
    result |= pieceAttacks<color>(pos.pieceAt(from).type, from, occupied);
  }

  return result;
}

template <GenGoal goal, bool pinHV, bool pinDA>
inline Move *generateMovesFromPieces(const Position &pos, Move *moves,
                                     Bitboard pieces, Bitboard specialMask,
                                     const GenCache &cache) {

  while (pieces) {
    Square from = pieces.popLsb();
    PieceType type = pos.pieceAt(from).type;
    Bitboard attacks = pieceAttacks(type, from, cache.occupied) & specialMask;
    if constexpr (pinHV)
      attacks &= rookMask(from);
    if constexpr (pinDA)
      attacks &= bishopMask(from);

    if constexpr (goal == QUIETS || goal == ALL) {
      Bitboard quiets = attacks & cache.empty;
      while (quiets) {
        *moves++ = Move(from, quiets.popLsb());
      }
    }

    if constexpr (goal == CAPTURES || goal == ALL) {
      Bitboard captures = attacks & cache.enemy;
      while (captures) {
        Square to = captures.popLsb();
        *moves++ = Move(from, to, CAPTURE);
      }
    }
  }

  return moves;
}

template <GenGoal goal, Color color>
Move *generatePawnMoves(const Position &pos, Move *moves,
                        const GenCache &cache) {
  constexpr Color enemyColor = EnemyColor<color>();

  constexpr Direction UP = color == WHITE ? NORTH : -NORTH;
  constexpr Direction UP_LEFT = color == WHITE ? NORTH_WEST : -NORTH_WEST;
  constexpr Direction UP_RIGHT = color == WHITE ? NORTH_EAST : -NORTH_EAST;
  constexpr Direction DOWN = -UP;
  constexpr Direction DOWN_LEFT = -UP_RIGHT;
  constexpr Direction DOWN_RIGHT = -UP_LEFT;

  constexpr Bitboard doublePushRank = (color == WHITE ? rank3 : rank6);
  constexpr Bitboard beforePromoRank = (color == WHITE ? rank7 : rank2);
  constexpr Bitboard notBeforePromo = ~beforePromoRank;

  Square epSquare = pos.getEpSquare();

  Bitboard pawns = pos.pieces<color, PAWN>();
  Bitboard pawnsBeforePromo = beforePromoRank & pawns;
  pawns &= notBeforePromo;

  if constexpr (goal == QUIETS || goal == ALL) {
    Bitboard singlePush = step<UP>(pawns & cache.moveH) & cache.empty;
    Bitboard doublePush = step<UP>(singlePush & doublePushRank) & cache.empty;

    singlePush &= cache.checkMask;
    doublePush &= cache.checkMask;

    while (singlePush) {
      Square to = singlePush.popLsb();
      *moves++ = Move(to + DOWN, to);
    }

    while (doublePush) {
      Square to = doublePush.popLsb();
      *moves++ = Move(to + (2 * DOWN), to, DOUBLE_PAWN_PUSH);
    }
  }

  if constexpr (goal == CAPTURES || goal == ALL) {
    Bitboard rightCapture =
        step<UP_RIGHT>(pawns & cache.moveD) & cache.enemy & cache.checkMask;
    Bitboard leftCapture =
        step<UP_LEFT>(pawns & cache.moveA) & cache.enemy & cache.checkMask;

    while (leftCapture) {
      Square to = leftCapture.popLsb();
      *moves++ = Move(to + DOWN_RIGHT, to, CAPTURE);
    }

    while (rightCapture) {
      Square to = rightCapture.popLsb();
      *moves++ = Move(to + DOWN_LEFT, to, CAPTURE);
    }
  }

  if (pawnsBeforePromo && (goal == PROMOTIONS || goal == ALL)) {

    Bitboard upPromo = step<UP>(pawnsBeforePromo & cache.moveH) & cache.empty &
                       cache.checkMask;
    while (upPromo) {
      Square to = upPromo.popLsb();
      moves = makePromo(moves, to + DOWN, to);
    }

    Bitboard rightPromo = step<UP_RIGHT>(pawnsBeforePromo & cache.moveD) &
                          cache.enemy & cache.checkMask;
    Bitboard leftPromo = step<UP_LEFT>(pawnsBeforePromo & cache.moveA) &
                         cache.enemy & cache.checkMask;
    while (rightPromo) {
      Square to = rightPromo.popLsb();
      moves = makePromoCapture(moves, to + DOWN_LEFT, to);
    }

    while (leftPromo) {
      Square to = leftPromo.popLsb();
      moves = makePromoCapture(moves, to + DOWN_RIGHT, to);
    }
  }

  if ((goal == CAPTURES || goal == ALL) && (epSquare != NULL_SQUARE) &&
      (pawnMask(pos.getEpSquare(), enemyColor) & pawns) &&
      cache.checkMask.get(epSquare + DOWN)) {
    Bitboard occ = pos.occupied();
    bool rightEp = (step<UP_RIGHT>(pawns & cache.moveD)).get(epSquare);
    bool leftEp = (step<UP_LEFT>(pawns & cache.moveA)).get(epSquare);

    if (rightEp) {
      Square attackingPawn = epSquare + DOWN_LEFT;
      Square attackedPawn = epSquare + DOWN;

      occ.clear(attackingPawn);
      occ.clear(attackedPawn);

      Bitboard rookAttack = rookAttacks(attackedPawn, occ);
      Bitboard bishopAttack = bishopAttacks(attackedPawn, occ);

      Bitboard rankAttack = rankMask(attackedPawn) & rookAttack;
      Bitboard diagAttack = diagonalMask(attackedPawn) & bishopAttack;
      Bitboard aDiagAttack = antiDiagonalMask(attackedPawn) & bishopAttack;

      Bitboard seenRankSliders =
          (pos.pieces<enemyColor, QUEEN>() | pos.pieces<enemyColor, ROOK>()) &
          rankAttack;
      Bitboard seenDiagSliders =
          (pos.pieces<enemyColor, QUEEN>() | pos.pieces<enemyColor, BISHOP>()) &
          diagAttack;
      Bitboard seenADiagSliders =
          (pos.pieces<enemyColor, QUEEN>() | pos.pieces<enemyColor, BISHOP>()) &
          aDiagAttack;

      bool pinRank = rankAttack.get(cache.king) && seenRankSliders;
      bool pinDiag = diagAttack.get(cache.king) && seenDiagSliders;
      bool pinADiag = aDiagAttack.get(cache.king) && seenADiagSliders;

      if (!(pinRank || pinDiag || pinADiag))
        *moves++ = Move(attackingPawn, epSquare, EP_CAPTURE);

      occ.set(attackingPawn);
      occ.set(attackedPawn);
    }

    if (leftEp) {
      Square attackingPawn = epSquare + DOWN_RIGHT;
      Square attackedPawn = epSquare + DOWN;

      occ.clear(attackingPawn);
      occ.clear(attackedPawn);

      Bitboard rookAttack = rookAttacks(attackedPawn, occ);
      Bitboard bishopAttack = bishopAttacks(attackedPawn, occ);

      Bitboard rankAttack = rankMask(attackedPawn) & rookAttack;
      Bitboard diagAttack = diagonalMask(attackedPawn) & bishopAttack;
      Bitboard aDiagAttack = antiDiagonalMask(attackedPawn) & bishopAttack;

      Bitboard seenRankSliders =
          (pos.pieces<enemyColor, QUEEN>() | pos.pieces<enemyColor, ROOK>()) &
          rankAttack;
      Bitboard seenDiagSliders =
          (pos.pieces<enemyColor, QUEEN>() | pos.pieces<enemyColor, BISHOP>()) &
          diagAttack;
      Bitboard seenADiagSliders =
          (pos.pieces<enemyColor, QUEEN>() | pos.pieces<enemyColor, BISHOP>()) &
          aDiagAttack;

      bool pinRank = rankAttack.get(cache.king) && seenRankSliders;
      bool pinDiag = diagAttack.get(cache.king) && seenDiagSliders;
      bool pinADiag = aDiagAttack.get(cache.king) && seenADiagSliders;

      if (!(pinRank || pinDiag || pinADiag))
        *moves++ = Move(attackingPawn, epSquare, EP_CAPTURE);

      occ.set(attackingPawn);
      occ.set(attackedPawn);
    }
  }

  return moves;
}

template <GenGoal goal>
inline Move *generateKingMoves(const Position &pos, Move *moves,
                               const GenCache &cache) {

  Bitboard kingTarget = kingMask(cache.king) & cache.safeSquares;

  if constexpr (goal == QUIETS || goal == ALL) {
    Bitboard kingQuiets = kingTarget & cache.empty;
    while (kingQuiets) {
      *moves++ = Move(cache.king, kingQuiets.popLsb());
    }
  }

  if constexpr (goal == CAPTURES || goal == ALL) {
    Bitboard kingCaptures = kingTarget & cache.enemy;
    while (kingCaptures) {
      Square to = kingCaptures.popLsb();
      *moves++ = Move(cache.king, to, CAPTURE);
    }
  }

  return moves;
}

inline Bitboard generateCheckMask(const Position &pos, const GenCache &cache) {
  unsigned int checks = cache.checkers.popCount();
  if (checks == 0) {
    return 0xffffffffffffffffULL;
  } else if (checks == 1) {
    Square checker = cache.checkers.lsb();
    PieceType type = pos.pieceAt(checker).type;
    if (type == ROOK || type == BISHOP || type == QUEEN) {
      return cache.checkers | commonRay[cache.king][checker];
    } else {
      return cache.checkers;
    }
  } else {
    return 0;
  }
}

template <GenGoal goal>
inline Move *generateSliderAndJumpMoves(const Position &pos, Move *moves,
                                        Bitboard pieces,
                                        const GenCache &cache) {
  Bitboard pinnedHV = cache.pinHV & pieces;
  Bitboard pinnedDA = cache.pinDA & pieces;
  pieces &= ~(pinnedHV | pinnedDA);

  moves = generateMovesFromPieces<goal, false, false>(pos, moves, pieces,
                                                      cache.checkMask, cache);

  moves = generateMovesFromPieces<goal, true, false>(
      pos, moves, pinnedHV, cache.checkMask & cache.pinHV, cache);

  moves = generateMovesFromPieces<goal, false, true>(
      pos, moves, pinnedDA, cache.checkMask & cache.pinDA, cache);

  return moves;
}

template <GenGoal goal, Color color>
Move *generateMoves(const Position &pos, Move *moves, GenCache &cache) {

  constexpr Color enemyColor = EnemyColor<color>();

  // Caching all the required data for movegen
  if constexpr (goal == INIT) {

    cache.king = pos.pieces<color, KING>().lsb();
    assert(cache.king != NULL_SQUARE);

    cache.friendlyPieces = pos.friendly<color>();
    cache.empty = pos.empty();
    cache.enemy = pos.enemy<color>();
    cache.occupied = pos.occupied();
    cache.checkers = getAttackers<color>(pos, cache.king);

    // Generating safe squares (not attacked by the opponent)
    cache.occupied.clear(cache.king);
    cache.safeSquares = ~getAttackedSquares<enemyColor>(pos, cache.occupied);
    cache.occupied.set(cache.king);

    // Generating check mask
    cache.checkMask = generateCheckMask(pos, cache);

    // Generating pin mask
    Bitboard seenSquares = pieceAttacks<QUEEN>(cache.king, cache.occupied);
    Bitboard possiblePins = seenSquares & cache.friendlyPieces;

    cache.occupied ^= possiblePins;

    Bitboard possiblePinners =
        (pieceAttacks<QUEEN>(cache.king, cache.occupied) ^ seenSquares) &
        cache.enemy;
    Bitboard pinners =
        ((pieceAttacks<ROOK>(cache.king, cache.occupied) & pos.pieces<ROOK>()) |
         (pieceAttacks<BISHOP>(cache.king, cache.occupied) &
          pos.pieces<BISHOP>()) |
         (pieceAttacks<QUEEN>(cache.king, cache.occupied) &
          pos.pieces<QUEEN>())) &
        possiblePinners;

    while (pinners) {
      Square pinner = pinners.popLsb();
      LineType type = lineType[cache.king][pinner];
      switch (type) {
      case HORIZONTAL:
        cache.pinH |= commonRay[cache.king][pinner] | pinner;
        break;
      case VERTICAL:
        cache.pinV |= commonRay[cache.king][pinner] | pinner;
        break;
      case DIAGONAL:
        cache.pinD |= commonRay[cache.king][pinner] | pinner;
        break;
      case ANTI_DIAGONAL:
        cache.pinA |= commonRay[cache.king][pinner] | pinner;
        break;
      }
    }

    cache.pinHV = cache.pinH | cache.pinV;
    cache.pinDA = cache.pinD | cache.pinA;

    cache.pinH &= cache.friendlyPieces;
    cache.pinV &= cache.friendlyPieces;
    cache.pinD &= cache.friendlyPieces;
    cache.pinA &= cache.friendlyPieces;

    cache.moveH = ~(cache.pinV | cache.pinD | cache.pinA);
    cache.moveV = ~(cache.pinH | cache.pinD | cache.pinA);
    cache.moveD = ~(cache.pinH | cache.pinV | cache.pinA);
    cache.moveA = ~(cache.pinH | cache.pinV | cache.pinD);

    cache.occupied ^= possiblePins;
  }

  // Generating king moves
  moves = generateKingMoves<goal>(pos, moves, cache);

  // If we are in a double check, only king moves are legal
  if (cache.checkMask == 0)
    return moves;

  // Generating pawn moves
  moves = generatePawnMoves<goal, color>(pos, moves, cache);

  // Generating knight and slider moves
  Bitboard sliderAndJumperPieces = cache.friendlyPieces & ~pos.pieces<PAWN>();
  sliderAndJumperPieces.clear(cache.king);

  moves = generateSliderAndJumpMoves<goal>(pos, moves, sliderAndJumperPieces,
                                           cache);

  // Generating castling moves
  if constexpr (goal == QUIETS || goal == ALL) {
    if constexpr (color == WHITE) {
      if (pos.getCastleRight(WK_MASK) &&
          (cache.safeSquares & WK_CASTLE_SAFE) == WK_CASTLE_SAFE &&
          (cache.empty & WK_CASTLE_EMPTY) == WK_CASTLE_EMPTY) {

        *moves++ = Move(E1, G1, KING_CASTLE);
      }

      if (pos.getCastleRight(WQ_MASK) &&
          (cache.safeSquares & WQ_CASTLE_SAFE) == WQ_CASTLE_SAFE &&
          (cache.empty & WQ_CASTLE_EMPTY) == WQ_CASTLE_EMPTY) {

        *moves++ = Move(E1, C1, QUEEN_CASTLE);
      }
    } else {
      if (pos.getCastleRight(BK_MASK) &&
          (cache.safeSquares & BK_CASTLE_SAFE) == BK_CASTLE_SAFE &&
          (cache.empty & BK_CASTLE_EMPTY) == BK_CASTLE_EMPTY) {

        *moves++ = Move(E8, G8, KING_CASTLE);
      }

      if (pos.getCastleRight(BQ_MASK) &&
          (cache.safeSquares & BQ_CASTLE_SAFE) == BQ_CASTLE_SAFE &&
          (cache.empty & BQ_CASTLE_EMPTY) == BQ_CASTLE_EMPTY) {

        *moves++ = Move(E8, C8, QUEEN_CASTLE);
      }
    }
  }

  return moves;
}

template <GenGoal goal>
Move *generateMoves(const Position &pos, Move *moves, GenCache &cache) {
  if (pos.getSideToMove() == WHITE) {
    return generateMoves<goal, WHITE>(pos, moves, cache);
  } else {
    return generateMoves<goal, BLACK>(pos, moves, cache);
  }
}

Move *generateMoves(GenGoal goal, const Position &pos, Move *moves,
                    GenCache &cache) {
  switch (goal) {
  case ALL:
    return generateMoves<ALL>(pos, moves, cache);
  case INIT:
    return generateMoves<INIT>(pos, moves, cache);
  case PROMOTIONS:
    return generateMoves<PROMOTIONS>(pos, moves, cache);
  case CAPTURES:
    return generateMoves<CAPTURES>(pos, moves, cache);
  case QUIETS:
    return generateMoves<QUIETS>(pos, moves, cache);
  }
}
