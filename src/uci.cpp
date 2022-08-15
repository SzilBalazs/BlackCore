#include <sstream>
#include <vector>
#include "uci.h"
#include "tt.h"
#include "position.h"
#include "movegen.h"

Move stringToMove(const Position& pos, const std::string &s) {
    Move moves[200];
    Move *endPtr = generateMoves(pos, moves, false);
    for (Move *m = moves; m != endPtr; m+=1) {
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
        }

        else if (command == "quit") {
            break;
        }

        else if (command == "stop") {

        }

        else if (command == "ucinewgame") {
            ttClear();
        }

        else if (command == "setoption") {
            if (tokens.size() >= 4) {
                if (tokens[1] == "Hash") {
                    ttResize(std::stoi(tokens[3]));
                }
            }
        }

        else if (command == "position" || command == "pos") {
            if (tokens[0] == "fen") {
                std::string fen;
                for (int i=1;i<tokens.size() && tokens[i] != "moves";i++) {
                    fen += tokens[i] + ' ';
                }
                pos.loadPositionFromFen(fen);
            } else if (tokens[0] == "startpos") {
                pos.loadPositionFromFen(STARTING_FEN);
            }

            if (line.find("moves") != std::string::npos) {
                int i = 0;
                bool move=false;
                while (i < tokens.size()) {
                    if (tokens[i] == "moves") move=true;
                    else if (move) {
                        pos.makeMove(stringToMove(pos, tokens[i]));
                    }
                    i++;
                }
            }
        }

        else if (command == "go") {

        }

        else if (command == "d") {
            pos.display();
        }
    }

}