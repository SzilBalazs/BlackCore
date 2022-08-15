#include <sstream>
#include <vector>
#include "uci.h"
#include "tt.h"
#include "position.h"

void uciLoop() {
    // Identifying ourselves
    out("id", "name", "BlackCoreV0");
    out("id", "author", "SzilBalazs");

    // We tell the GUI what options we have
    out("option", "name", "Hash", "type", "spin", "default", 128, "min", 1, "max", 1024);

    // We have sent all the parameters
    out("uciok");

    Position pos = {STARTING_FEN};

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

        }

        else if (command == "position") {

        }

        else if (command == "go") {

        }

        else if (command == "d") {
            pos.display();
        }
    }

}