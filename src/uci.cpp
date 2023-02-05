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

#include "uci.h"
#include "bench.h"
#include "fathom/src/tbprobe.h"
#include "position.h"
#include "search.h"
#include "timeman.h"
#include "tt.h"
#include <iomanip>
#include <sstream>
#include <vector>

bool guiCommunication = false;

Move stringToMove(const Position &pos, const std::string &s) {
    Square from = stringToSquare(s.substr(0, 2));
    Square to = stringToSquare(s.substr(2, 2));
    Piece piece = pos.pieceAt(from);
    Piece capturedPiece = pos.pieceAt(to);
    unsigned int flags = QUIET_MOVE;
    if (!capturedPiece.isNull())
        flags = CAPTURE_FLAG;

    if (s.size() == 5) {
        PieceType type = charToPiece(s[4]).type;
        switch (type) {
            case QUEEN:
                return {from, to, flags | PROMO_QUEEN};
            case ROOK:
                return {from, to, flags | PROMO_ROOK};
            case BISHOP:
                return {from, to, flags | PROMO_BISHOP};
            case KNIGHT:
                return {from, to, flags | PROMO_KNIGHT};
            default:
                out("Invalid move!");
                return {};
        }
    }

    if (piece.type == PAWN && pos.getEpSquare() == to) {
        flags = EP_CAPTURE;
    } else if (piece.type == PAWN && std::abs((long) squareToRank(from) - (long) squareToRank(to)) == 2) {
        flags = DOUBLE_PAWN_PUSH;
    } else if (piece.type == KING && squareToFile(from) == 4) {
        if (squareToFile(to) == 6) {
            flags = KING_CASTLE;
        } else if (squareToFile(to) == 2) {
            flags = QUEEN_CASTLE;
        }
    }

    return {from, to, flags};
}

std::string scoreColor(Score score) {
    if (WORST_MATE < std::abs(score))
        return asciiColor(11);
    else if (TB_WORST_WIN < std::abs(score))
        return asciiColor(141);
    else if (10 <= score)
        return asciiColor(42);
    else if (score <= -10)
        return asciiColor(9);
    else
        return asciiColor(255);
}

std::string formatInt(U64 n) {
    std::string str;
    if (n < 1000) {
        str = std::to_string(n);
    } else if (n < 10000 * 1000) {
        str = std::to_string(U64(n / 1000)) + "K";
    } else {
        str = std::to_string(U64(n / (1000 * 1000))) + "M";
    }
    return str;
}

std::string formatMillis(U64 milli) {
    std::string str;
    if (milli < 1000) {
        str = std::to_string(milli) + "ms";
    } else if (milli < (1000 * 60)) {
        str = std::to_string(milli / 1000) + "s";
    } else {
        str = std::to_string(milli / (1000 * 60)) + "m";
    }
    return str;
}

std::string formatScore(Score score) {
    std::string str = score >= 0 ? "+" : "-";

    if (std::abs(score) > WORST_MATE) {
        str += 'M' + std::to_string(MATE_VALUE - std::abs(score));
    } else if (std::abs(score) > TB_WORST_WIN) {
        str += "TB " + std::to_string(TB_WIN_SCORE - std::abs(score));
    } else {
        double x = std::abs(double(score) / 100);
        std::string xstr = std::to_string(x);
        int fPoints = -1;
        for (char c : xstr) {
            if (c == '.') {
                fPoints = 0;
            }

            if (fPoints > 2) break;

            str += c;

            if (fPoints != -1) fPoints++;
        }
    }

    return str;
}

void printCurrMove(Depth depth, int index, Move move) {
    if (guiCommunication)
        out("info", "depth", int(depth), "currmove", move, "currmovenumber", index + 1);
    else
        std::cout << asciiColor(252) << "Searching " << move << "...\r\u001b[0m" << std::flush;
}

void printNewDepth(Depth depth, Depth selectiveDepth, U64 nodes, int hashFull, U64 tbHits, Score score, const std::string &scoreStr, U64 time, U64 nps, const std::string &pv) {
    if (guiCommunication) {
        out("info", "depth", int(depth), "seldepth", int(selectiveDepth), "nodes", nodes, "hashfull", hashFull, "tbhits", tbHits, "score", scoreStr, "time",
            time, "nps", nps, "pv", pv);
    } else {
        std::string lineColor = depth & 1 ? asciiColor(247) : asciiColor(251);
        std::string coloredPV = asciiColor(39);
        bool bestMove = true;
        for (char c : pv) {
            if (bestMove && c == ' ') {
                bestMove = false;
                coloredPV += asciiColor(255);
            }
            coloredPV += c;
        }

        std::string d = std::to_string(depth) + '/' + std::to_string(selectiveDepth);

        std::cout << lineColor;
        std::cout << "   " << std::setw(5)
                  << d << "" << scoreColor(score) << std::setw(10)
                  << formatScore(score) << "" << lineColor << std::setw(10)
                  << formatInt(nodes) << " " << std::setw(7)
                  << formatMillis(time) << "   " << std::setw(6)
                  << formatInt(nps) << "   " << std::setw(5) << std::fixed << std::setprecision(1)
                  << double(hashFull) / 10 << "% " << std::setw(9)
                  << formatInt(tbHits) << "   "
                  << coloredPV << std::endl;
        std::cout << "\u001b[0m";
    }
}

void playGame(Position &position) {
    while (true) {
#ifdef _WIN64
        system("cls");
#else
        system("clear");
#endif
        position.display();

        Move moves[200];
        Move *endMove = generateMoves(position, moves, false);

        if (endMove == moves) {
            std::cout << "Checkmate!" << std::endl;
            break;
        } else if (position.isRepetition()) {
            std::cout << "Repetition draw!" << std::endl;
            break;
        } else if (position.getMove50() >= 99) {
            std::cout << "Fifty move draw!" << std::endl;
            break;
        }

        std::cout << "Make a move: " << std::flush;
        std::string moveStr;
        std::getline(std::cin, moveStr);
        if (moveStr.empty())
            break;
        Move move = stringToMove(position, moveStr);
        bool match = false;
        for (auto i : moves) {
            if (i == move) {
                match = true;
                break;
            }
        }
        if (!match) continue;
        position.makeMove(move);
        if (move.isCapture() || position.pieceAt(move.getTo()).type == PAWN) {
            position.resetStack();
        }
    }
}

void uciInitProtocol() {

    guiCommunication = true;

    // Identifying ourselves
#ifdef VERSION
    out("id", "name", "BlackCore", VERSION);
#else
    out("id", "name", "BlackCore");
#endif

    out("id", "author", "SzilBalazs");

    // Tell the GUI what options we have
    out("option", "name", "Hash", "type", "spin", "default", 32, "min", 1, "max", 4096);
    out("option", "name", "Threads", "type", "spin", "default", 1, "min", 1, "max", 64);
    out("option", "name", "EvalFile", "type", "string", "default", "corenet.bin");
    out("option", "name", "SyzygyPath", "type", "string", "default", "<none>");
    out("option", "name", "Move Overhead", "type", "spin", "default", 5, "min", 0, "max", 10000);

#ifdef TUNE
    tuneOut("DELTA_MARGIN", 400, 200, 500);
    tuneOut("RAZOR_MARGIN", 130, 50, 200);
    tuneOut("RFP_DEPTH", 5, 3, 10);
    tuneOut("RFP_DEPTH_MULTIPLIER", 70, 30, 200);
    tuneOut("RFP_IMPROVING_MULTIPLIER", 80, 30, 200);
    tuneOut("NULL_MOVE_DEPTH", 3, 1, 6);
    tuneOut("NULL_MOVE_BASE_R", 4, 2, 6);
    tuneOut("NULL_MOVE_R_SCALE", 5, 2, 10);
    tuneOut("LMR_DEPTH", 4, 2, 10);
    tuneOut("LMR_INDEX", 3, 1, 10);
    tuneOut("LMP_DEPTH", 4, 1, 10);
    tuneOut("LMP_MOVES", 5, 1, 10);
    tuneOut("ASPIRATION_DEPTH", 9, 5, 20);
    tuneOut("ASPIRATION_DELTA", 30, 10, 100);
    tuneOut("SEE_MARGIN", 0, 0, 200);
    tuneOut("PAWN_VALUE", 150, 100, 200);
    tuneOut("KNIGHT_VALUE", 750, 300, 1000);
    tuneOut("BISHOP_VALUE", 850, 300, 1000);
    tuneOut("ROOK_VALUE", 800, 300, 1000);
    tuneOut("QUEEN_VALUE", 1000, 500, 1500);
    tuneOut("LMR_BASE", 10, 1, 30);
    tuneOut("LMR_SCALE", 17, 10, 40);
#endif

    // We have sent all the parameters
    out("uciok");
}

void uciLoop() {
    // Initialize stuff
    initSearch();
    ttResize(32);

    Position pos = {STARTING_FEN};
    int threadCount = 1;

    while (true) {
        std::string line, command, token;
        std::getline(std::cin, line);

        std::stringstream ss(line);

        std::getline(ss, command, ' ');

        std::vector<std::string> tokens;
        while (std::getline(ss, token, ' ')) {
            tokens.emplace_back(token);
        }

        if (command == "uci") {
            uciInitProtocol();
        } else if (command == "isready") {
            out("readyok");
        } else if (command == "quit") {
            joinThreads(false);
            tb_free();
            break;
        } else if (command == "stop") {
            joinThreads(false);
        } else if (command == "ucinewgame") {
            ttClear();
        } else if (command == "setoption") {
            if (tokens.size() >= 4) {
                if (tokens[1] == "Hash") {
                    ttResize(std::stoi(tokens[3]));
                } else if (tokens[1] == "Move" && tokens[2] == "Overhead") {
                    MOVE_OVERHEAD = std::stoi(tokens[4]);
                } else if (tokens[1] == "Ponder") {

                } else if (tokens[1] == "Threads") {
                    threadCount = std::stoi(tokens[3]);
                } else if (tokens[1] == "SyzygyPath") {
                    tb_init(tokens[3].c_str());
                } else if (tokens[1] == "EvalFile") {
                    NNUE::EVALFILE = tokens[3];
                    NNUE::init();
                } else {
                    out("info", "string", "Unknown value", tokens[1]);
                }
            }
        } else if (command == "position" || command == "pos") {

            if (tokens[0] == "fen") {
                std::string fen;
                for (unsigned int i = 1; i < tokens.size() && tokens[i] != "moves"; i++) {
                    fen += tokens[i] + ' ';
                }
                pos.loadPositionFromFen(fen);
            } else if (tokens[0] == "startpos") {
                pos.loadPositionFromFen(STARTING_FEN);
            }

            if (line.find("moves") != std::string::npos) {
                unsigned int i = 0;
                bool move = false;
                while (i < tokens.size()) {
                    if (tokens[i] == "moves")
                        move = true;
                    else if (move) {
                        Move m = stringToMove(pos, tokens[i]);
                        pos.makeMove(m);
                        if (m.isCapture() || pos.pieceAt(m.getTo()).type == PAWN) {
                            pos.resetStack();
                        }
                    }
                    i++;
                }
            }

            pos.getState()->accumulator.refresh(pos);

        } else if (command == "go") {

            SearchInfo searchInfo;

            for (unsigned int i = 0; i < tokens.size(); i += 2) {
                if (tokens[i] == "wtime") {
                    searchInfo.wtime = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "btime") {
                    searchInfo.btime = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "winc") {
                    searchInfo.winc = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "binc") {
                    searchInfo.binc = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "movestogo") {
                    searchInfo.movestogo = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "depth") {
                    searchInfo.maxDepth = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "movetime") {
                    searchInfo.movetime = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "nodes") {
                    searchInfo.maxNodes = std::stoi(tokens[i + 1]);
                } else if (tokens[i] == "infinite") {
                }
            }
            if (!guiCommunication) {
                out("   Depth    ", "Score    ", "Nodes   ", "Time     ", "NPS   ", "Hash%  ", "TB Hits  ", "Principal Variation");
            }

            startSearch(searchInfo, pos, threadCount);

        } else if (command == "d" || command == "display") {
            pos.display();
        } else if (command == "e" || command == "eval") {
            pos.displayEval();
        } else if (command == "moves") {
            Move moves[200];
            Move *movesEnd = generateMoves(pos, moves, false);
            for (Move *it = moves; it != movesEnd; it++) {
                out(*it);
            }
        } else if (command == "perft") {
            out("Total nodes:", perft<true>(pos, std::stoi(tokens[0])));
        } else if (command == "play") {
            playGame(pos);
        }
    }
    ttFree();
}