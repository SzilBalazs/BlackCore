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

#ifndef BLACKCORE_MOVEGEN_H
#define BLACKCORE_MOVEGEN_H

#include "move.h"
#include "move_ordering.h"
#include "position.h"

enum GenGoal { ALL, INIT, PROMOTIONS, CAPTURES, QUIETS };

struct GenCache {
  Square king;
  Bitboard friendlyPieces, empty, enemy, occupied, checkers, safeSquares,
      checkMask, pinH, pinV, pinD, pinA, pinHV, pinDA, moveH, moveV, moveD,
      moveA, sliderAndJumperPieces;
};

bool isPseudoLegal(const Position &pos, Move move);

template <Color color>
inline Bitboard getAttackers(const Position &pos, Square square) {
  Bitboard occupied = pos.occupied();
  Bitboard enemy = pos.enemy<color>();
  return ((pawnMask(square, color) & pos.pieces<PAWN>()) |
          (pieceAttacks<KNIGHT>(square, occupied) & pos.pieces<KNIGHT>()) |
          (pieceAttacks<BISHOP>(square, occupied) & pos.pieces<BISHOP>()) |
          (pieceAttacks<ROOK>(square, occupied) & pos.pieces<ROOK>()) |
          (pieceAttacks<QUEEN>(square, occupied) & pos.pieces<QUEEN>())) &
         enemy;
}

inline Bitboard getAttackers(const Position &pos, Square square) {
  if (pos.getSideToMove() == WHITE)
    return getAttackers<WHITE>(pos, square);
  else
    return getAttackers<BLACK>(pos, square);
}

Move *generateMoves(GenGoal goal, const Position &pos, Move *moves,
                    GenCache &cache);

inline Move *generateAllMoves(const Position &pos, Move *moves) {
  GenCache cache;
  return generateMoves(ALL, pos, moves, cache);
}

/*struct MoveList {
  Move moves[200];
  Move *movesEnd;
  unsigned int index;

  Score scores[200];

  unsigned int count;

  MoveList(const Position &pos, Ply ply, bool capturesOnly) {
    movesEnd = generateAllMoves(pos, moves);
    index = 0;
    count = movesEnd - moves;

    // Scoring moves
    if (capturesOnly) {
      for (unsigned int i = 0; i < count; i++) {
        scores[i] = scoreQMove(pos, moves[i]);
      }
    } else {
      for (unsigned int i = 0; i < count; i++) {
        scores[i] = scoreMove(pos, moves[i], ply);
      }
    }
  }

  inline bool empty() const { return index == count; }

  inline Move nextMove() {
    unsigned int best = index;
    for (unsigned int i = index; i < count; i++) {
      if (scores[i] > scores[best]) {
        best = i;
      }
    }
    std::swap(moves[index], moves[best]);
    std::swap(scores[index], scores[best]);

    return moves[index++];
  }
};*/

#endif // BLACKCORE_MOVEGEN_H
