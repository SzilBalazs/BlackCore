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

#ifndef BLACKCORE_UCI_H
#define BLACKCORE_UCI_H

#include "constants.h"
#include <iostream>

struct SearchInfo {
    U64 wtime = 0, btime = 0, winc = 0, binc = 0, movestogo = 0, movetime = 0, maxNodes = 0;
    Depth maxDepth = MAX_PLY;
    bool uciMode = true;
};

namespace BlackCore {
    inline void _out() {
        std::cout << std::endl;
    }

    template<typename T, typename... Args>
    inline void _out(T a, Args... args) {
        std::cout << " " << a;
        _out(args...);
    }
} // namespace BlackCore

template<typename T, typename... Args>
inline void out(T a, Args... args) {
    std::cout << a;
    BlackCore::_out(args...);
}

void uciLoop();

#endif //BLACKCORE_UCI_H
