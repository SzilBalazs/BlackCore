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

bool Move::operator==(Move a) const {
    return (data & 0xFFFF) == (a.data & 0xFFFF);
}

bool Move::operator!=(Move a) const {
    return (data & 0xFFFF) != (a.data & 0xFFFF);
}

Move::operator bool() const {
    return !isNull();
}

bool Move::isCapture() const {
    return isFlag(CAPTURE_FLAG);
}

bool Move::isPromo() const {
    return isFlag(PROMO_FLAG);
}

bool Move::isSpecial1() const {
    return isFlag(SPECIAL1_FLAG);
}

bool Move::isSpecial2() const {
    return isFlag(SPECIAL2_FLAG);
}

bool Move::isQuiet() const {
    return !isCapture();
}

std::string Move::str() const {
    std::string token;
    if (isPromo()) {
        if (!isSpecial1() && !isSpecial2()) token += "n";
        else if (isSpecial1() && !isSpecial2()) token += "b";
        else if (!isSpecial1() && isSpecial2()) token += "r";
        else token += "q";
    }
    return formatSquare(getFrom()) + formatSquare(getTo()) + token;
}

