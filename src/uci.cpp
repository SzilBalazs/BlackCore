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

#include "uci.h"
#include "bench.h"
#include "eval.h"
#include "position.h"
#include "search.h"
#include "timeman.h"
#include "tt.h"
#include <sstream>
#include <vector>

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

void uciLoop() {
    // Identifying ourselves
#ifdef VERSION
    out("id", "name", "BlackCore", VERSION);
#else
    out("id", "name", "BlackCore");
#endif

    out("id", "author", "SzilBalazs");

    // We tell the GUI what options we have
    out("option", "name", "Hash", "type", "spin", "default", 32, "min", 1, "max", 4096);
    out("option", "name", "Threads", "type", "spin", "default", 1, "min", 1, "max", 1);
    out("option", "name", "Ponder", "type", "check", "default", "false");
    out("option", "name", "Move Overhead", "type", "spin", "default", 10, "min", 0, "max", 10000);

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

    ttResize(32);

    // We have sent all the parameters
    out("uciok");

    // Only now we initialize stuff
    initSearch();

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

        if (command == "isready") {
            out("readyok");
        } else if (command == "quit") {
            joinThreads(false);
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
                } else {
#ifdef TUNE
                    if (tokens[1] == "DELTA_MARGIN") {
                        DELTA_MARGIN = std::stoi(tokens[3]);
                    } else if (tokens[1] == "RAZOR_MARGIN") {
                        RAZOR_MARGIN = std::stoi(tokens[3]);
                    } else if (tokens[1] == "RFP_DEPTH") {
                        RFP_DEPTH = std::stoi(tokens[3]);
                    } else if (tokens[1] == "RFP_DEPTH_MULTIPLIER") {
                        RFP_DEPTH_MULTIPLIER = std::stoi(tokens[3]);
                    } else if (tokens[1] == "RFP_IMPROVING_MULTIPLIER") {
                        RFP_IMPROVING_MULTIPLIER = std::stoi(tokens[3]);
                    } else if (tokens[1] == "NULL_MOVE_DEPTH") {
                        NULL_MOVE_DEPTH = std::stoi(tokens[3]);
                    } else if (tokens[1] == "NULL_MOVE_BASE_R") {
                        NULL_MOVE_BASE_R = std::stoi(tokens[3]);
                    } else if (tokens[1] == "NULL_MOVE_R_SCALE") {
                        NULL_MOVE_R_SCALE = std::stoi(tokens[3]);
                    } else if (tokens[1] == "LMR_DEPTH") {
                        LMR_DEPTH = std::stoi(tokens[3]);
                    } else if (tokens[1] == "LMR_INDEX") {
                        LMR_INDEX = std::stoi(tokens[3]);
                    } else if (tokens[1] == "LMP_DEPTH") {
                        LMP_DEPTH = std::stoi(tokens[3]);
                    } else if (tokens[1] == "LMP_MOVES") {
                        LMP_MOVES = std::stoi(tokens[3]);
                    } else if (tokens[1] == "ASPIRATION_DEPTH") {
                        ASPIRATION_DEPTH = std::stoi(tokens[3]);
                    } else if (tokens[1] == "ASPIRATION_DELTA") {
                        ASPIRATION_DELTA = std::stoi(tokens[3]);
                    } else if (tokens[1] == "SEE_MARGIN") {
                        SEE_MARGIN = std::stoi(tokens[3]);
                    } else if (tokens[1] == "PAWN_VALUE") {
                        PIECE_VALUES[PAWN] = std::stoi(tokens[3]);
                    } else if (tokens[1] == "KNIGHT_VALUE") {
                        PIECE_VALUES[KNIGHT] = std::stoi(tokens[3]);
                    } else if (tokens[1] == "BISHOP_VALUE") {
                        PIECE_VALUES[BISHOP] = std::stoi(tokens[3]);
                    } else if (tokens[1] == "ROOK_VALUE") {
                        PIECE_VALUES[ROOK] = std::stoi(tokens[3]);
                    } else if (tokens[1] == "QUEEN_VALUE") {
                        PIECE_VALUES[QUEEN] = std::stoi(tokens[3]);
                    } else if (tokens[1] == "LMR_BASE") {
                        LMR_BASE = double(std::stoi(tokens[3])) / 10;
                    } else if (tokens[1] == "LMR_SCALE") {
                        LMR_SCALE = double(std::stoi(tokens[3])) / 10;
                    }
#endif
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
                        pos.makeMove(stringToMove(pos, tokens[i]));
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
        }
    }
    ttFree();
}