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

#include "bench.h"
#include "tools.h"
#include "uci.h"

#ifdef _WIN64
#include <windows.h>
#endif


int main(int argc, char **argv) {

    srand(RANDOM_SEED);

#ifdef _WIN64
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD f;
    GetConsoleMode(hConsole, &f);
    SetConsoleMode(hConsole, f | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

    std::string mode;
    if (argc >= 2) {
        mode = std::string(argv[1]);
    }

    if (mode == "bench") {
        testSearch();
    } else if (mode == "perft") {
        testPerft();
    } else if (mode == "filter") {
        processPlain(argv[2]);
    } else {
        uciLoop();
    }

    return 0;
}
