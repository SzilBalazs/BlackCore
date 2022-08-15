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
#include "uci.h"
#include "tt.h"
#include "position.h"
#include "movegen.h"

Move stringToMove(const Position &pos, const std::string &s) {
    Move moves[200];
    Move *endPtr = generateMoves(pos, moves, false);
    for (Move *m = moves; m != endPtr; m += 1) {
        if (m->str() == s) return *m;
    }
}

void uciLoop() {
    // Identifying ourselves
    out("id", "name", "BlackCoreV0");

    out("id", "author", "SzilBalazs");

    // We tell the GUI what options we have
    out("option", "name", "Hash", "type", "spin", "default", 128, "min", 1, "max", 1024);
    ttResize(128);

    // We have sent all the parameters
    out("uciok");

    Position pos;

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
            break;
        } else if (command == "stop") {

        } else if (command == "ucinewgame") {
            ttClear();
        } else if (command == "setoption") {
            if (tokens.size() >= 4) {
                if (tokens[1] == "Hash") {
                    ttResize(std::stoi(tokens[3]));
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

        } else if (command == "d") {
            pos.display();
        }
    }

}