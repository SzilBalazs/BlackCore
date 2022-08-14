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

#ifndef BLACKCORE_TT_H
#define BLACKCORE_TT_H

#include "constants.h"
#include "move.h"

enum EntryFlag : uint16_t {
    NONE = 0, EXACT = 1, ALPHA = 2, BETA = 3
};

struct TTEntry {    // Total: 24 bytes
    U64 hash;       // 8 bytes
    Depth depth;    // 4 bytes
    Score eval;     // 4 bytes
    Move hashMove;  // 4 bytes
    EntryFlag flag; // 1 bytes
    uint16_t age;    // 1 bytes
};

struct TTBucket {               // 64 bytes
    TTEntry entryA;             // 24 bytes
    TTEntry entryB;             // 24 bytes
    U64 _padding1, _padding2;   // 16 bytes
};

struct TTable {
    TTBucket *table;
    unsigned int bucketCount;
    U64 mask;
};

void ttResize(unsigned int MBSize);

void ttClear();

Score ttProbe(U64 hash, Depth depth, Score alpha, Score beta);

void ttSave(U64 hash, Depth depth, Score eval, EntryFlag flag, Move bestMove);

Move getHashMove(U64 hash);

#endif //BLACKCORE_TT_H
