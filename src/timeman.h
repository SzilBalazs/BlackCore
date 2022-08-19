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

#ifndef BLACKCORE_TIMEMAN_H
#define BLACKCORE_TIMEMAN_H

#include "constants.h"

extern unsigned int MOVE_OVERHEAD;

void startSearch(U64 time, U64 inc, U64 movestogo, U64 movetime);

bool shouldEnd();

void stopSearch();

U64 getSearchTime();

U64 getNps();

#endif //BLACKCORE_TIMEMAN_H
