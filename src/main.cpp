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

#include "bench.h"
#include "uci.h"
#include <iostream>

int main(int argc, char **argv) {

    srand(RANDOM_SEED);
    std::string mode;
    if (argc >= 2) {
        mode = std::string(argv[1]);
    }

    if (mode.empty()) {
        std::cin >> mode;
    }

    if (mode == "bench") {
        testSearch();
    } else if (mode == "perft") {
        testPerft();
    } else if (mode == "uci") {
        uciLoop();
    } else {
        std::cout << "Invalid option! (uci/bench/perft)" << std::endl;
    }

    return 0;
}
