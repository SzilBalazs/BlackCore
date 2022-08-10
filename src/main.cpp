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
#include <chrono>
#include "movegen.h"
#include "utils.h"

int main() {
    srand(6);
    initBitboard();

    int bestSeed;
    long bestTime = 9999999999;

    for (int seed = 0; seed <= 200; seed++) {
        srand(seed);
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        findMagics(rookAttackTable, rookMagics, ROOK);
        findMagics(bishopAttackTable, bishopMagics, BISHOP);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        long t = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
        if (t < bestTime) {
            bestTime = t;
            bestSeed = seed;
        }

        std::cout << "seed = " << seed << " Time it took: "
                  << t << " Best time so far: " << bestTime << "(" << bestSeed << ")" << std::endl;
    }


    return 0;
}
