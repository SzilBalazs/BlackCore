// BlackCore is a chess engine
// Copyright (c) 2022-2023 SzilBalazs
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "constants.h"
#include "move.h"

enum EntryFlag : uint8_t {
    TT_NONE = 0,
    TT_EXACT = 1,

    // UPPERBOUND
    TT_ALPHA = 2,

    // LOWERBOUND
    TT_BETA = 3
};

struct TTEntry {    // Total: 16 bytes
    U64 hash;       // 8 bytes
    Score eval;     // 4 bytes
    Move hashMove;  // 2 bytes
    Depth depth;    // 1 byte
    EntryFlag flag; // 1 byte
};

struct TTable {
    TTEntry *table;
    unsigned int bucketCount;
    U64 mask;
};

void ttResize(unsigned int MBSize);

void ttClear();

void ttFree();

TTEntry ttProbe(U64 hash, Ply ply, bool &ttHit);

void ttSave(U64 hash, Depth depth, Score eval, EntryFlag flag, Move bestMove, Ply ply);

int getTTFull();

Move getHashMove(U64 hash);

void ttPrefetch(U64 hash);
