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

#include "move.h"
#include "utils.h"

std::string Move::str() const {
    std::string token;
    if (isPromo()) {
        if (!isSpecial1() && !isSpecial2())
            token += "n";
        else if (!isSpecial1() && isSpecial2())
            token += "b";
        else if (isSpecial1() && !isSpecial2())
            token += "r";
        else
            token += "q";
    }
    return formatSquare(getFrom()) + formatSquare(getTo()) + token;
}

std::ostream &operator<<(std::ostream &os, const Move &move) {
    os << move.str();
    return os;
}
