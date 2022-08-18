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

#ifndef BLACKCORE_EVAL_H
#define BLACKCORE_EVAL_H

#include "constants.h"
#include "position.h"

constexpr Score TEMPO_SCORE = 10;

constexpr Score ROOK_OPEN_BONUS = 15;
constexpr Score ROOK_HOPEN_BONUS = 5;

constexpr Score PIECE_VALUES[6] = {0, 90, 300, 350, 520, 1000};

Score eval(const Position &pos);


#endif //BLACKCORE_EVAL_H
