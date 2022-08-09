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

using std::cout, std::string;

void Position::clearSquare(Square square) {
    Piece piece = pieceAt(square);

    pieceBB[piece.type].clear(square);
    allPieceBB[piece.color].clear(square);
    board[square] = {};
}

void Position::setSquare(Square square, Piece piece) {
    clearSquare(square);

    pieceBB[piece.type].set(square);
    allPieceBB[piece.color].set(square);
    board[square] = piece;
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

    stm = WHITE;
    epSquare = NULL_SQUARE;
    castlingRights = 0;
}

void Position::displayBoard() {
    std::vector<string> text;

    if (epSquare != NULL_SQUARE)
        text.emplace_back(string("En passant square: ") + formatSquare(epSquare));
    text.emplace_back(string("Castle rights: "));
    text.emplace_back(string("Side to move: ") + string(stm == WHITE ? "White" : "Black"));
    // TODO FEN, hash key, full-half move counter

    cout << "    A   B   C   D   E   F   G   H  \n";
    for (int i = 8; i >= 1; i--) {
        cout << "  +---+---+---+---+---+---+---+---+";
        if (i <= 7 && !text.empty()) {
            cout << "        " << text.back();
            text.pop_back();
        }
        cout << "\n" << i << " |";
        for (int j = 1; j <= 8; j++) {
            cout << " " << pieceToChar(pieceAt(Square((i - 1) * 8 + (j - 1)))) << " |";
        }
        if (i <= 7 && !text.empty()) {
            cout << "        " << text.back();
            text.pop_back();
        }
        cout << "\n";
    }
    cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
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

}

Position::Position() {
    clearPosition();
}

Position::Position(const std::string &fen) {
    loadPositionFromFen(fen);
}