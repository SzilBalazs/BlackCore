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

#ifndef BLACKCORE_POSITION_H
#define BLACKCORE_POSITION_H

#include "bitboard.h"
#include "move.h"
#include "nnue.h"
#include "utils.h"
#include <vector>

extern U64 nodeCount;

struct BoardState {
  Color stm = COLOR_EMPTY;
  Square epSquare = NULL_SQUARE;
  unsigned char castlingRights = 0;
  U64 hash = 0;

  Piece capturedPiece = {};

  BoardState *lastIrreversibleMove = nullptr;

  NNUE::Accumulator accumulator = {};

  constexpr BoardState() = default;
};

struct RawState {
  Piece board[64];
  Bitboard pieceBB[6] = {0}, allPieceBB[2] = {0};
  Color stm = COLOR_EMPTY;
  Square epSquare = NULL_SQUARE;
  unsigned char castlingRights = 0;
};

struct StateStack {

  BoardState stateStart[1000];
  BoardState *currState;

  StateStack() {
    currState = stateStart;
    currState->lastIrreversibleMove = currState;
  }

  inline void push(BoardState newState) {
    newState.lastIrreversibleMove = currState->lastIrreversibleMove;
    newState.accumulator.loadAccumulator(currState->accumulator);
    currState++;
    *currState = newState;
  }

  inline void pop() { currState--; }

  inline BoardState *top() const { return currState; }

  inline void clear() {
    currState = stateStart;
    *currState = {};
  }

  inline Ply getMove50() const {
    return currState - currState->lastIrreversibleMove;
  }
};

#define state states.top()

class Position {
public:
  constexpr Piece pieceAt(Square square) const { return board[square]; }

  // This will be mostly used with constant color and type so this will result a
  // nicer code pieces<{ROOK, WHITE}>() --> pieces<WHITE, ROOK>()
  template <Color color, PieceType type> constexpr Bitboard pieces() const {
    return pieceBB[type] & allPieceBB[color];
  }

  template <Color color> constexpr Bitboard pieces(PieceType type) const {
    return pieceBB[type] & allPieceBB[color];
  }

  template <PieceType type> constexpr Bitboard pieces(Color color) const {
    return pieceBB[type] & allPieceBB[color];
  }

  constexpr Bitboard pieces(Color color, PieceType type) const {
    return pieceBB[type] & allPieceBB[color];
  }

  template <PieceType type> constexpr Bitboard pieces() const {
    return pieceBB[type];
  }

  constexpr Bitboard pieces(PieceType type) const { return pieceBB[type]; }

  template <Color color> constexpr Bitboard friendly() const {
    return allPieceBB[color];
  }

  constexpr Bitboard friendly(Color color) const { return allPieceBB[color]; }

  template <Color color> constexpr Bitboard enemy() const {
    return allPieceBB[EnemyColor<color>()];
  }

  template <Color color> constexpr Bitboard enemyOrEmpty() const {
    return ~friendly<color>();
  }

  constexpr Bitboard enemyOrEmpty(Color color) const {
    return ~friendly(color);
  }

  inline Bitboard occupied() const {
    return allPieceBB[WHITE] | allPieceBB[BLACK];
  }

  inline Bitboard empty() const { return ~occupied(); }

  inline Color getSideToMove() const { return state->stm; }

  inline Square getEpSquare() const { return state->epSquare; }

  inline bool getCastleRight(unsigned char castleRight) const {
    return castleRight & state->castlingRights;
  }

  inline unsigned char getCastlingRights() const {
    return state->castlingRights;
  }

  inline BoardState *getState() { return state; }

  inline BoardState *getState() const { return state; }

  inline U64 getHash() const { return state->hash; }

  inline Ply getMove50() const { return states.getMove50(); }

  inline void makeMove(Move move);

  inline void undoMove(Move move);

  void makeNullMove();

  void undoNullMove();

  bool isRepetition();

  void display() const;

  void displayEval();

  void loadPositionFromFen(const std::string &fen);

  void loadPositionFromRawState(const RawState &rawState);

  RawState getRawState();

  Position();

  Position(const std::string &fen);

private:
  template <bool updateAccumulator> void clearSquare(Square square);

  template <bool updateAccumulator> void setSquare(Square square, Piece piece);

  template <bool updateAccumulator> void movePiece(Square from, Square to);

  inline void setCastleRight(unsigned char castleRight) {
    state->castlingRights |= castleRight;
  }

  inline void removeCastleRight(unsigned char castleRight) {
    state->castlingRights &= ~castleRight;
  }

  void clearPosition();

  template <Color color> void makeMove(Move move);

  template <Color color> void undoMove(Move move);

  Piece board[64];

  Bitboard pieceBB[6], allPieceBB[2];

  StateStack states;
};

template <bool updateAccumulator> void Position::clearSquare(Square square) {
  if (pieceAt(square).isNull())
    return;

  Piece piece = pieceAt(square);

  pieceBB[piece.type].clear(square);
  allPieceBB[piece.color].clear(square);

  board[square] = {};

  state->hash ^= pieceRandTable[12 * square + 6 * piece.color + piece.type];

  if constexpr (updateAccumulator) {
    state->accumulator.removeFeature(
        NNUE::getAccumulatorIndex(piece.color, piece.type, square));
  }
}

template <bool updateAccumulator>
void Position::setSquare(Square square, Piece piece) {
  if (!pieceAt(square).isNull()) {
    Piece p = pieceAt(square);

    pieceBB[p.type].clear(square);
    allPieceBB[p.color].clear(square);

    state->hash ^= pieceRandTable[12 * square + 6 * p.color + p.type];

    if constexpr (updateAccumulator) {
      state->accumulator.removeFeature(
          NNUE::getAccumulatorIndex(p.color, p.type, square));
    }
  }

  pieceBB[piece.type].set(square);
  allPieceBB[piece.color].set(square);
  board[square] = piece;

  state->hash ^= pieceRandTable[12 * square + 6 * piece.color + piece.type];

  if constexpr (updateAccumulator) {
    state->accumulator.addFeature(
        NNUE::getAccumulatorIndex(piece.color, piece.type, square));
  }
}

template <bool updateAccumulator>
void Position::movePiece(Square from, Square to) {
  setSquare<updateAccumulator>(to, pieceAt(from));
  clearSquare<updateAccumulator>(from);
}

template <Color color> void Position::makeMove(Move move) {
  nodeCount++;

  BoardState newState;

  constexpr Color enemyColor = EnemyColor<color>();
  constexpr Direction UP = color == WHITE ? NORTH : -NORTH;
  constexpr Direction DOWN = -UP;

  Square from = move.getFrom();
  Square to = move.getTo();

  if (move.equalFlag(EP_CAPTURE)) {
    newState.capturedPiece = {PAWN, enemyColor};
  } else {
    newState.capturedPiece = pieceAt(to);
  }
  newState.castlingRights = state->castlingRights;
  newState.stm = enemyColor;
  newState.hash = state->hash ^ *blackRand;

  // Removing ep from hash
  if (state->epSquare != NULL_SQUARE) {
    newState.hash ^= epRandTable[squareToFile(state->epSquare)];
  }

  if (move.equalFlag(DOUBLE_PAWN_PUSH)) {
    newState.epSquare = from + UP;
    newState.hash ^= epRandTable[squareToFile(newState.epSquare)];
  } else {
    newState.epSquare = NULL_SQUARE;
  }

  states.push(newState);

  if (move.equalFlag(EP_CAPTURE)) {
    clearSquare<true>(to + DOWN);
  }

  if (move.isCapture() || pieceAt(from).type == PAWN) {
    state->lastIrreversibleMove = state;
  }

  // Removing castling rights
  state->hash ^= castlingRandTable[state->castlingRights];
  if (getCastleRight(WK_MASK) && (from == E1 || from == H1 || to == H1)) {
    removeCastleRight(WK_MASK);
  }
  if (getCastleRight(WQ_MASK) && (from == E1 || from == A1 || to == A1)) {
    removeCastleRight(WQ_MASK);
  }
  if (getCastleRight(BK_MASK) && (from == E8 || from == H8 || to == H8)) {
    removeCastleRight(BK_MASK);
  }
  if (getCastleRight(BQ_MASK) && (from == E8 || from == A8 || to == A8)) {
    removeCastleRight(BQ_MASK);
  }
  state->hash ^= castlingRandTable[state->castlingRights];

  // Moving rook in case of a castle
  if (move.equalFlag(KING_CASTLE)) {
    if constexpr (color == WHITE) {
      movePiece<true>(H1, F1);
    } else {
      movePiece<true>(H8, F8);
    }
  } else if (move.equalFlag(QUEEN_CASTLE)) {
    if constexpr (color == WHITE) {
      movePiece<true>(A1, D1);
    } else {
      movePiece<true>(A8, D8);
    }
  }

  movePiece<true>(from, to);

  if (move.isFlag(PROMO_FLAG)) {
    Piece piece = {PIECE_EMPTY, color};
    if (move.equalFlag(PROMO_KNIGHT) || move.equalFlag(PROMO_CAPTURE_KNIGHT)) {
      piece.type = KNIGHT;
    } else if (move.equalFlag(PROMO_BISHOP) ||
               move.equalFlag(PROMO_CAPTURE_BISHOP)) {
      piece.type = BISHOP;
    } else if (move.equalFlag(PROMO_ROOK) ||
               move.equalFlag(PROMO_CAPTURE_ROOK)) {
      piece.type = ROOK;
    } else if (move.equalFlag(PROMO_QUEEN) ||
               move.equalFlag(PROMO_CAPTURE_QUEEN)) {
      piece.type = QUEEN;
    }
    setSquare<true>(to, piece);
  }
}

template <Color color> void Position::undoMove(Move move) {

  constexpr Color enemyColor = EnemyColor<color>();
  constexpr Direction UP = enemyColor == WHITE ? NORTH : -NORTH;
  constexpr Direction DOWN = -UP;

  Square from = move.getFrom();
  Square to = move.getTo();

  movePiece<false>(to, from);

  if (move.equalFlag(KING_CASTLE)) {
    if constexpr (enemyColor == WHITE) {
      movePiece<false>(F1, H1);
    } else {
      movePiece<false>(F8, H8);
    }
  } else if (move.equalFlag(QUEEN_CASTLE)) {
    if constexpr (enemyColor == WHITE) {
      movePiece<false>(D1, A1);
    } else {
      movePiece<false>(D8, A8);
    }
  }

  if (move.equalFlag(EP_CAPTURE))
    setSquare<false>(to + DOWN, state->capturedPiece);
  else if (move.isCapture())
    setSquare<false>(to, state->capturedPiece);

  if (move.isPromo())
    setSquare<false>(from, {PAWN, enemyColor});

  states.pop();
}

inline void Position::makeMove(Move move) {
  if (getSideToMove() == WHITE)
    makeMove<WHITE>(move);
  else
    makeMove<BLACK>(move);
}

inline void Position::undoMove(Move move) {
  if (getSideToMove() == WHITE)
    undoMove<WHITE>(move);
  else
    undoMove<BLACK>(move);
}

#undef state

#endif // BLACKCORE_POSITION_H
