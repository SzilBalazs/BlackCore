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

#include "movegen.h"
#include "utils.h"

int main() {
    initBitboard();
    std::string fen;
    getline(std::cin, fen);
    Position pos = {fen};
    pos.display();
    Move moves[200];
    Move *movesEnd = generateMoves(pos, moves);
    std::cout << movesEnd - moves << " pseudo legal moves found: " << std::endl;
    for (Move *it = moves; it != movesEnd; it++) {
        std::cout << it->str() << std::endl;
    }
    return 0;
}
