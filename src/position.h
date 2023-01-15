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

#ifndef BLACKCORE_POSITION_H
#define BLACKCORE_POSITION_H

#include "bitboard.h"
#include "move.h"
#include "nnue.h"
#include "utils.h"
#include <vector>

// Stores the state of a board.
struct BoardState {
    Color stm = COLOR_EMPTY;          // Side to move
    Square epSquare = NULL_SQUARE;    // En passant square
    unsigned char castlingRights = 0; // Castling rights
    U64 hash = 0;                     // Zobrist hash

    Piece capturedPiece = {}; // Piece captured in the last move

    BoardState *lastIrreversibleMove = nullptr; // Pointer to the last irreversible state.

    NNUE::Accumulator accumulator = {}; // NNUE accumulator

    constexpr BoardState() = default;

    inline void load(BoardState &state) {
        stm = state.stm;
        epSquare = state.epSquare;
        castlingRights = state.castlingRights;
        hash = state.hash;
        capturedPiece = state.capturedPiece;
    }
};

struct StateStack {

    BoardState stateStart[500];
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

    inline void pop() {
        currState--;
    }

    [[nodiscard]] inline BoardState *top() const {
        return currState;
    }

    inline void clear() {
        currState = stateStart;
        *currState = {};
    }

    inline void reset() {
        stateStart->load(*currState);
        stateStart->lastIrreversibleMove = stateStart;
        stateStart->accumulator.loadAccumulator(currState->accumulator);
        currState = stateStart;
    }

    [[nodiscard]] inline Ply getMove50() const {
        return currState - currState->lastIrreversibleMove;
    }
};

#define state states.top()

class Position {
public:
    // Returns the piece at a square.
    [[nodiscard]] constexpr Piece pieceAt(Square square) const {
        return board[square];
    }

    // Returns the bitboard of all the pieces of a color and type.
    template<Color color, PieceType type>
    [[nodiscard]] constexpr Bitboard pieces() const {
        return pieceBB[type] & allPieceBB[color];
    }

    // Returns the bitboard of all the pieces of a color and type.
    template<Color color>
    [[nodiscard]] constexpr Bitboard pieces(PieceType type) const {
        return pieceBB[type] & allPieceBB[color];
    }

    // Returns the bitboard of all the pieces of a color and type.
    template<PieceType type>
    [[nodiscard]] constexpr Bitboard pieces(Color color) const {
        return pieceBB[type] & allPieceBB[color];
    }

    // Returns the bitboard of all the pieces of a color and type.
    [[nodiscard]] constexpr Bitboard pieces(Color color, PieceType type) const {
        return pieceBB[type] & allPieceBB[color];
    }

    // Returns the bitboard of all the pieces of a type.
    template<PieceType type>
    [[nodiscard]] constexpr Bitboard pieces() const {
        return pieceBB[type];
    }

    // Returns the bitboard of all the pieces of a type.
    [[nodiscard]] constexpr Bitboard pieces(PieceType type) const {
        return pieceBB[type];
    }

    // Returns the bitboard of all the friendly/same color pieces.
    template<Color color>
    [[nodiscard]] constexpr Bitboard friendly() const {
        return allPieceBB[color];
    }

    // Returns the bitboard of all the friendly/same color pieces.
    [[nodiscard]] constexpr Bitboard friendly(Color color) const {
        return allPieceBB[color];
    }

    // Returns the bitboard of all the enemy/different color pieces.
    template<Color color>
    [[nodiscard]] constexpr Bitboard enemy() const {
        return allPieceBB[EnemyColor<color>()];
    }

    // Returns the bitboard of all the enemy or empty pieces.
    template<Color color>
    [[nodiscard]] constexpr Bitboard enemyOrEmpty() const {
        return ~friendly<color>();
    }

    // Returns the bitboard of all occupied squares.
    [[nodiscard]] inline Bitboard occupied() const {
        return allPieceBB[WHITE] | allPieceBB[BLACK];
    }

    // Returns the bitboard of all empty squares.
    [[nodiscard]] inline Bitboard empty() const {
        return ~occupied();
    }

    // Returns the color that makes the next move.
    [[nodiscard]] inline Color getSideToMove() const {
        return state->stm;
    }

    // Returns the en-passant square.
    [[nodiscard]] inline Square getEpSquare() const {
        return state->epSquare;
    }

    // Returns true if the castleRight is legal.
    [[nodiscard]] inline bool getCastleRight(unsigned char castleRight) const {
        return castleRight & state->castlingRights;
    }

    // Returns the castling rights.
    [[nodiscard]] inline unsigned char getCastlingRights() const {
        return state->castlingRights;
    }

    // Returns the current board state.
    [[nodiscard]] inline BoardState *getState() {
        return state;
    }

    // Returns the current board state.
    [[nodiscard]] inline BoardState *getState() const {
        return state;
    }

    // Resets the state stack. Most commonly used after an irreversible move.
    inline void resetStack() {
        states.reset();
    }

    // Returns the Zobrist hash of the position.
    [[nodiscard]] inline U64 getHash() const {
        return state->hash;
    }

    // Returns the half-move counter of the position.
    [[nodiscard]] inline Ply getMove50() const {
        return states.getMove50();
    }

    inline void makeMove(Move move);

    inline void undoMove(Move move);

    void makeNullMove();

    void undoNullMove();

    bool isRepetition();

    bool isPseudoLegal(Move move);

    void display() const;

    void displayEval();

    void loadPositionFromFen(const std::string &fen);

    void loadFromPosition(const Position &position);

    Position();

    Position(const std::string &fen);

    StateStack states;

private:
    template<bool updateAccumulator>
    void clearSquare(Square square);

    template<bool updateAccumulator>
    void setSquare(Square square, Piece piece);

    template<bool updateAccumulator>
    void movePiece(Square from, Square to);

    // Sets a castling right.
    inline void setCastleRight(unsigned char castleRight) {
        state->castlingRights |= castleRight;
    }

    // Removes a castling right.
    inline void removeCastleRight(unsigned char castleRight) {
        state->castlingRights &= ~castleRight;
    }

    void clearPosition();

    template<Color color>
    void makeMove(Move move);

    template<Color color>
    void undoMove(Move move);

    /*
     * BlackCore uses a hybrid approach for board representation.
     *
     * Bitboards most importantly allows us to loop through a type of piece.
     * For example RANK2 & WHITE & PAWN
     */
    Piece board[64];                    // Mailbox board representation
    Bitboard pieceBB[6], allPieceBB[2]; // Bitboard board representation
};

// Clears a square and updates hash & NNUE accumulator.
template<bool updateAccumulator>
void Position::clearSquare(Square square) {
    if (pieceAt(square).isNull())
        return;

    Piece piece = pieceAt(square);

    pieceBB[piece.type].clear(square);
    allPieceBB[piece.color].clear(square);

    board[square] = {};

    state->hash ^= pieceRandTable[12 * square + 6 * piece.color + piece.type];

    if constexpr (updateAccumulator) {
        state->accumulator.removeFeature(piece.color, piece.type, square);
    }
}

// Sets a square and updates hash & NNUE accumulator.
template<bool updateAccumulator>
void Position::setSquare(Square square, Piece piece) {
    if (!pieceAt(square).isNull()) {
        Piece p = pieceAt(square);

        pieceBB[p.type].clear(square);
        allPieceBB[p.color].clear(square);

        state->hash ^= pieceRandTable[12 * square + 6 * p.color + p.type];

        if constexpr (updateAccumulator) {
            state->accumulator.removeFeature(p.color, p.type, square);
        }
    }

    pieceBB[piece.type].set(square);
    allPieceBB[piece.color].set(square);
    board[square] = piece;

    state->hash ^= pieceRandTable[12 * square + 6 * piece.color + piece.type];

    if constexpr (updateAccumulator) {
        state->accumulator.addFeature(piece.color, piece.type, square);
    }
}

// Moves a piece.
template<bool updateAccumulator>
void Position::movePiece(Square from, Square to) {
    setSquare<updateAccumulator>(to, pieceAt(from));
    clearSquare<updateAccumulator>(from);
}

// Makes a move. Doesn't check for legality!
template<Color color>
void Position::makeMove(Move move) {

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
        } else if (move.equalFlag(PROMO_BISHOP) || move.equalFlag(PROMO_CAPTURE_BISHOP)) {
            piece.type = BISHOP;
        } else if (move.equalFlag(PROMO_ROOK) || move.equalFlag(PROMO_CAPTURE_ROOK)) {
            piece.type = ROOK;
        } else if (move.equalFlag(PROMO_QUEEN) || move.equalFlag(PROMO_CAPTURE_QUEEN)) {
            piece.type = QUEEN;
        }
        setSquare<true>(to, piece);
    }
}

// Undo a move. Doesn't check for legality!
template<Color color>
void Position::undoMove(Move move) {

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

// Makes a move. Doesn't check for legality!
inline void Position::makeMove(Move move) {
    if (getSideToMove() == WHITE)
        makeMove<WHITE>(move);
    else
        makeMove<BLACK>(move);
}

// Undo a move. Doesn't check for legality!
inline void Position::undoMove(Move move) {
    if (getSideToMove() == WHITE)
        undoMove<WHITE>(move);
    else
        undoMove<BLACK>(move);
}

#undef state

#endif //BLACKCORE_POSITION_H
