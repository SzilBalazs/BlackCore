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

#include <iostream>

inline void _out() {
    std::cout << std::endl;
}

template<typename T, typename... Args>
inline void _out(T a, Args... args) {
    std::cout << " " << a;
    _out(args...);
}

template<typename T, typename... Args>
inline void out(T a, Args... args) {
    std::cout << a;
    _out(args...);
}

inline void tuneOut(const std::string &name, int value, int min, int max) {
    out("option", "name", name, "type", "spin", "default", value, "min", min, "max", max);
}

void uciLoop();

#endif //BLACKCORE_UCI_H
