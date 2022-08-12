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

#include <iostream>
#include <vector>
#include <sstream>
#include "position.h"

#define state states.top()

using std::cout, std::string;

void Position::clearSquare(Square square) {
    PieceType type = pieceAt(square).type;

    pieceBB[type].clear(square);
    allPieceBB[WHITE].clear(square);
    allPieceBB[BLACK].clear(square);
    board[square] = {};
}

void Position::setSquare(Square square, Piece piece) {
    clearSquare(square);

    pieceBB[piece.type].set(square);
    allPieceBB[piece.color].set(square);
    board[square] = piece;
}

void Position::movePiece(Square from, Square to) {
    setSquare(to, pieceAt(from));
    clearSquare(from);
}

void Position::clearPosition() {
    for (auto &i : pieceBB) {
        i = 0;
    }

    for (Square square = A1; square < 64; square += 1) {
        board[square] = {};
    }

    allPieceBB[WHITE] = 0;
    allPieceBB[BLACK] = 0;

    states.push({});

    state->stm = WHITE;
    state->epSquare = NULL_SQUARE;
    state->castlingRights = 0;
    state->capturedPiece = {};
}

void Position::display() {
    std::vector<string> text;
    if (getEpSquare() != NULL_SQUARE)
        text.emplace_back(string("En passant square: ") + formatSquare(getEpSquare()));
    string cr;
    if (getCastleRight(WK_MASK)) cr += 'K';
    if (getCastleRight(WQ_MASK)) cr += 'Q';
    if (getCastleRight(BK_MASK)) cr += 'k';
    if (getCastleRight(BQ_MASK)) cr += 'q';
    if (cr.empty()) cr = "None";
    text.emplace_back(string("Castling rights: ") + cr);
    text.emplace_back(string("Side to move: ") + string(getSideToMove() == WHITE ? "White" : "Black"));
    // TODO FEN, hash key, full-half move counter

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
    cout << "   +---+---+---+---+---+---+---+---+\n\n" << std::endl;
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
            setSquare(square, charToPiece(c));
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
            break;
        default:
            assert(1);
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
                    assert(1);
            }
        }
    }

    ss >> state->epSquare;
}

Position::Position() {
    clearPosition();
}

Position::Position(const std::string &fen) {
    loadPositionFromFen(fen);
}