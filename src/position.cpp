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

#include "position.h"
#include "eval.h"

#include <iomanip>
#include <iostream>
#include <vector>

#define state states.top()

using std::cout, std::string;

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

void Position::undoNullMove() {
    states.pop();
}

bool Position::isRepetition() {
    // TODO we can make this faster, because we only have to check every second ply
    for (BoardState *ptr = state->lastIrreversibleMove; ptr != state; ptr++) {
        if (state->hash == ptr->hash) {
            return true;
        }
    }
    return false;
}

void Position::display() const {

    std::vector<string> text;
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
    // TODO FEN, full-half move counter

    cout << "\n     A   B   C   D   E   F   G   H  \n";
    for (int i = 8; i >= 1; i--) {
        cout << "   +---+---+---+---+---+---+---+---+";
        if (i <= 7 && !text.empty()) {
            cout << "        " << text.back();
            text.pop_back();
        }
        cout << "\n " << i << " |";
        for (int j = 1; j <= 8; j++) {
            cout << " " << pieceToChar(pieceAt(Square((i - 1) * 8 + (j - 1)))) << " |";
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

    state->accumulator.refresh(*this);
}

void Position::loadPositionFromRawState(const RawState &rawState) {
    clearPosition();
    state->stm = rawState.stm;
    state->epSquare = rawState.epSquare;
    state->castlingRights = rawState.castlingRights;
    state->hash = rawState.hash;
    allPieceBB[WHITE] = rawState.allPieceBB[WHITE];
    allPieceBB[BLACK] = rawState.allPieceBB[BLACK];
    
    for (int i = 0; i < 6; i++) {
        pieceBB[i] = rawState.pieceBB[i];
    }

    for (Square sq = A1; sq < 64; sq += 1) {
        board[sq] = rawState.board[sq];
    }

    state->accumulator.refresh(*this);
}

RawState Position::getRawState() const {
    RawState rawState;
    rawState.stm = getSideToMove();
    rawState.epSquare = getEpSquare();
    rawState.castlingRights = getCastlingRights();
    rawState.hash = getHash();
    rawState.allPieceBB[WHITE] = allPieceBB[WHITE];
    rawState.allPieceBB[BLACK] = allPieceBB[BLACK];
    
    for (int i = 0; i < 6; i++) {
        rawState.pieceBB[i] = pieceBB[i];
    }

    for (Square sq = A1; sq < 64; sq += 1) {
        rawState.board[sq] = board[sq];
    }

    return rawState;
}

Position::Position() {
    clearPosition();
}

Position::Position(const std::string &fen) {
    loadPositionFromFen(fen);
}