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

#include <sstream>
#include <vector>
#include <thread>
#include "uci.h"
#include "tt.h"
#include "search.h"
#include "timeman.h"
#include "position.h"

Move stringToMove(const Position &pos, const std::string &s) {
    Square from = stringToSquare(s.substr(0, 2));
    Square to = stringToSquare(s.substr(2, 2));
    Piece piece = pos.pieceAt(from);
    Piece capturedPiece = pos.pieceAt(to);
    Color enemyColor = piece.color == WHITE ? BLACK : WHITE;
    unsigned int flags = QUIET_MOVE;
    if (!capturedPiece.isNull())
        flags = CAPTURE_FLAG;

    if (s.size() == 5) {
        PieceType type = charToPiece(s[4]).type;
        switch (type) {
            case QUEEN:
                return {from, to, flags | PROMO_QUEEN, capturedPiece};
            case ROOK:
                return {from, to, flags | PROMO_ROOK, capturedPiece};
            case BISHOP:
                return {from, to, flags | PROMO_BISHOP, capturedPiece};
            case KNIGHT:
                return {from, to, flags | PROMO_KNIGHT, capturedPiece};
            default:
                out("Invalid move!");
                return {};
        }
    }

    if (piece.type == PAWN && pos.getEpSquare() == to) {
        flags = EP_CAPTURE;
        capturedPiece = {PAWN, enemyColor};
    } else if (piece.type == PAWN && std::abs((long) squareToRank(from) - squareToRank(to)) == 2) {
        flags = DOUBLE_PAWN_PUSH;
    } else if (piece.type == KING && squareToFile(from) == 4) {
        if (squareToFile(to) == 6) {
            flags = KING_CASTLE;
        } else if (squareToFile(to) == 2) {
            flags = QUEEN_CASTLE;
        }
    }

    return {from, to, flags, capturedPiece};
}

void uciLoop() {
    // Identifying ourselves
    out("id", "name", "BlackCoreV0");

    out("id", "author", "SzilBalazs");

    // We tell the GUI what options we have
    out("option", "name", "Hash", "type", "spin", "default", 16, "min", 1, "max", 1024);
    out("option", "name", "Threads", "type", "spin", "default", 1, "min", 1, "max", 1);
    out("option", "name", "Ponder", "type", "check", "default", "false");
    out("option", "name", "Move Overhead", "type", "spin", "default", 50, "min", 0, "max", 10000);

    ttResize(16);

    // We have sent all the parameters
    out("uciok");

    Position pos;
    std::thread searchThread;

    while (true) {
        std::string line, command, token;
        std::getline(std::cin, line);

        std::stringstream ss(line);

        std::getline(ss, command, ' ');

        std::vector<std::string> tokens;
        while (std::getline(ss, token, ' ')) {
            tokens.emplace_back(token);
        }

        if (command == "isready") {
            out("readyok");
        } else if (command == "quit") {
            if (searchThread.joinable())
                searchThread.join();
            break;
        } else if (command == "stop") {
            stopSearch();
            if (searchThread.joinable())
                searchThread.join();
        } else if (command == "ucinewgame") {
            ttClear();
        } else if (command == "setoption") {
            if (tokens.size() >= 4) {
                if (tokens[1] == "Hash") {
                    ttResize(std::stoi(tokens[3]));
                } else if (tokens[1] == "Move" && tokens[2] == "Overhead") {
                    MOVE_OVERHEAD = std::stoi(tokens[4]);
                }
            }
        } else if (command == "position" || command == "pos") {

            if (tokens[0] == "fen") {
                std::string fen;
                for (int i = 1; i < tokens.size() && tokens[i] != "moves"; i++) {
                    fen += tokens[i] + ' ';
                }
                pos.loadPositionFromFen(fen);
            } else if (tokens[0] == "startpos") {
                pos.loadPositionFromFen(STARTING_FEN);
            }

            if (line.find("moves") != std::string::npos) {
                int i = 0;
                bool move = false;
                while (i < tokens.size()) {
                    if (tokens[i] == "moves") move = true;
                    else if (move) {
                        pos.makeMove(stringToMove(pos, tokens[i]));
                    }
                    i++;
                }
            }

        } else if (command == "go") {

            if (searchThread.joinable())
                searchThread.join();

            U64 wtime = 0, btime = 0, winc = 0, binc = 0, movestogo = 50, depth = 64, movetime = 0;

            for (int i = 0; i < tokens.size(); i += 2) {
                if (tokens[i] == "wtime") {
                    wtime = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "btime") {
                    btime = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "winc") {
                    winc = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "binc") {
                    binc = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "movestogo") {
                    movestogo = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "depth") {
                    depth = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "movetime") {
                    movetime = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "infinite") {
                    depth = 64;
                }
            }

            // A little more time, so we can fill up the TT
            if (getHashMove(pos.getHash()).isNull()) {
                movestogo /= 2;
            }

            if (pos.getSideToMove() == WHITE)
                startSearch(wtime, winc, movestogo, movetime);
            else
                startSearch(btime, binc, movestogo, movetime);

            searchThread = std::thread(iterativeDeepening, pos, depth, true);

        } else if (command == "d") {
            pos.display();
        }
    }
}