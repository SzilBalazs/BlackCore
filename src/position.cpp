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

#include "position.h"
#include "eval.h"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <vector>

#define state states.top()

using std::cout, std::string;

// Clears the position.
void Position::clearPosition() {
    for (auto &i : pieceBB) {
        i = 0;
    }

    for (Square square = A1; square < 64; square += 1) {
        board[square] = {};
    }

    allPieceBB[WHITE] = 0;
    allPieceBB[BLACK] = 0;

    states.clear();
    state->lastIrreversibleMove = state;
}

// Makes a null move, which is used in null move pruning.
void Position::makeNullMove() {
    BoardState newState;

    newState.stm = state->stm == WHITE ? BLACK : WHITE;
    newState.castlingRights = state->castlingRights;
    newState.hash = state->hash ^ *blackRand;
    newState.lastIrreversibleMove = state->lastIrreversibleMove;

    if (state->epSquare != NULL_SQUARE) {
        newState.hash ^= epRandTable[squareToFile(state->epSquare)];
    }

    states.push(newState);
}

// Undo a null move.
void Position::undoNullMove() {
    states.pop();
}

// Returns true if one more instance of this position was found before.
bool Position::isRepetition() {
    for (BoardState *ptr = state->lastIrreversibleMove; ptr != state; ptr++) {
        if (state->hash == ptr->hash) {
            return true;
        }
    }
    return false;
}

// Displays the current position in the console.
void Position::display() const {

    std::vector<string> text;
    text.emplace_back(string("50-move draw counter: ") + std::to_string(getMove50()));
    text.emplace_back(string("Hash: ") + std::to_string(state->hash));

    if (getEpSquare() != NULL_SQUARE)
        text.emplace_back(string("En passant square: ") + formatSquare(getEpSquare()));
    string cr;
    if (getCastleRight(WK_MASK))
        cr += 'K';
    if (getCastleRight(WQ_MASK))
        cr += 'Q';
    if (getCastleRight(BK_MASK))
        cr += 'k';
    if (getCastleRight(BQ_MASK))
        cr += 'q';
    if (cr.empty())
        cr = "None";

    text.emplace_back(string("Castling rights: ") + cr);
    text.emplace_back(string("Side to move: ") + string(getSideToMove() == WHITE ? "White" : "Black"));

    cout << "\n     A   B   C   D   E   F   G   H  \n";
    for (int i = 8; i >= 1; i--) {
        cout << "   +---+---+---+---+---+---+---+---+";
        if (i <= 7 && !text.empty()) {
            cout << "        " << text.back();
            text.pop_back();
        }
        cout << "\n " << i << " |";
        for (int j = 1; j <= 8; j++) {
            Piece piece = pieceAt(Square((i - 1) * 8 + (j - 1)));
            cout << (piece.color == WHITE ? ASCII_WHITE_PIECE : (piece.color == BLACK ? ASCII_BLACK_PIECE : "")) << " " << pieceToChar(piece) << " \u001b[0m|";
        }
        if (i <= 7 && !text.empty()) {
            cout << "        " << text.back();
            text.pop_back();
        }
        cout << "\n";
    }
    cout << "   +---+---+---+---+---+---+---+---+\n\n"
         << std::endl;
}

// Displays the NNUE's take on the current position.
void Position::displayEval() {
    Score score = eval(*this);
    cout << "\n      A     B     C     D     E     F     G     H    \n";
    for (int i = 8; i >= 1; i--) {
        cout << "   +-----+-----+-----+-----+-----+-----+-----+-----+";
        cout << "\n " << i << " |";
        for (int j = 1; j <= 8; j++) {
            Square square = Square((i - 1) * 8 + (j - 1));
            cout << "  " << pieceToChar(pieceAt(square)) << "  |";
        }
        cout << "\n   |";
        for (int j = 1; j <= 8; j++) {
            Square square = Square((i - 1) * 8 + (j - 1));
            Piece piece = pieceAt(square);
            string evalStr = " ";
            if (!piece.isNull() && piece.type != KING) {
                clearSquare<true>(square);
                Score newScore = eval(*this);
                Score scoreDiff = score - newScore;
                evalStr = std::to_string(scoreDiff);
                setSquare<true>(square, piece);
            }
            cout << std::setw(5) << evalStr << "|";
        }
        cout << "\n";
    }
    cout << "   +-----+-----+-----+-----+-----+-----+-----+-----+\n"
         << std::endl;

    cout << "Eval: " << score << std::endl;
}

// Loads the board from a FEN.
void Position::loadPositionFromFen(const string &fen) {
    clearPosition();

    std::stringstream ss;
    string b;
    ss << fen;
    ss >> b;
    Square square = A8;
    for (char c : b) {
        if ('1' <= c && c <= '8') {
            square += c - '0';
        } else if (c == '/') {
            square -= 16;
        } else {
            setSquare<false>(square, charToPiece(c));
            square += 1;
        }
    }

    char c;
    ss >> c;
    switch (c) {
        case 'w':
            state->stm = WHITE;
            break;
        case 'b':
            state->stm = BLACK;
            state->hash ^= *blackRand;
            break;
        default:
            assert(0);
    }
    string cr;
    ss >> cr;
    if (cr[0] != '-') {
        for (char r : cr) {
            switch (r) {
                case 'K':
                    setCastleRight(WK_MASK);
                    break;
                case 'Q':
                    setCastleRight(WQ_MASK);
                    break;
                case 'k':
                    setCastleRight(BK_MASK);
                    break;
                case 'q':
                    setCastleRight(BQ_MASK);
                    break;
                default:
                    assert(0);
            }
        }
    }

    ss >> state->epSquare;

    state->hash ^= castlingRandTable[state->castlingRights];
    state->hash ^= epRandTable[squareToFile(state->epSquare)];

#ifndef DATA_FILTER
    state->accumulator.refresh(*this);
#endif
}

Bitboard Position::getAllAttackers(Square square, Bitboard occ) const {
    return (((pawnMasks[square][WHITE] | pawnMasks[square][BLACK]) & pieces<PAWN>()) |
            (pieceAttacks<KNIGHT>(square, occ) & pieces<KNIGHT>()) |
            (pieceAttacks<BISHOP>(square, occ) & pieces<BISHOP>()) |
            (pieceAttacks<ROOK>(square, occ) & pieces<ROOK>()) |
            (pieceAttacks<QUEEN>(square, occ) & pieces<QUEEN>())) &
           occ;
}

Bitboard Position::leastValuablePiece(Bitboard attackers, Color stm, PieceType &type) const {

    for (PieceType t : PIECE_TYPES_BY_VALUE) {
        Bitboard s = attackers & pieces(stm, t);
        if (s) {
            type = t;
            return s & -s.bb;
        }
    }
    return 0;
}

Position::Position() {
    clearPosition();
}

Position::Position(const std::string &fen) {
    loadPositionFromFen(fen);
}

void Position::loadFromPosition(const Position &position) {
    for (Square idx = A1; idx < 64; idx += 1) {
        board[idx] = position.pieceAt(idx);
    }

    for (Color color : {WHITE, BLACK}) {
        allPieceBB[color] = position.friendly(color);
    }

    for (PieceType type : {KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN}) {
        pieceBB[type] = position.pieces(type);
    }

    std::memcpy(states.stateStart, position.states.stateStart, sizeof(BoardState) * 500);
    states.currState = states.stateStart + (position.states.currState - position.states.stateStart);

    for (int idx = 0; idx < 500; idx++) {
        states.stateStart[idx].lastIrreversibleMove = states.stateStart + (position.states.stateStart[idx].lastIrreversibleMove - position.states.stateStart);
    }
}