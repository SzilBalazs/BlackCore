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

#include "tt.h"
#include <cstring>

#ifdef __linux__

#include <sys/mman.h>

#endif

TTable tt;

// Returns a candidate ttEntry corresponding to a hash.
TTEntry *getEntry(U64 hash) {
    return tt.table + (hash & tt.mask);
}

// Clears the transposition table.
void ttClear() {
    std::memset(tt.table, 0, tt.bucketCount * sizeof(TTEntry));
}

// Frees the space allocated for the transposition table.
void ttFree() {
    free(tt.table);
}

// Resizes the transposition table.
void ttResize(unsigned long long MBSize) {

    if (tt.bucketCount)
        ttFree();

    unsigned int i = 10;
    while ((1ULL << i) <= MBSize * 1024ULL * 1024ULL / sizeof(TTEntry))
        i++;

    tt.bucketCount = (1ULL << (i - 1));
    tt.mask = tt.bucketCount - 1ULL;

#ifdef __linux__
    // Allocate memory with 1MB alignment
    tt.table = static_cast<TTEntry *>(aligned_alloc((1ULL << 20), tt.bucketCount * sizeof(TTEntry)));

    // For reference see https://man7.org/linux/man-pages/man2/madvise.2.html on MADV HUGEPAGE
    madvise(tt.table, tt.bucketCount * sizeof(TTEntry), MADV_HUGEPAGE);
#else
    tt.table = (TTEntry *) malloc(tt.bucketCount * sizeof(TTEntry));
#endif

    ttClear();
}

// Probes a Zobrish hash and sets ttHit to true, if it succeeds.
TTEntry ttProbe(U64 hash, Ply ply, bool &ttHit) {
    TTEntry entry = *getEntry(hash);

    if (entry.hash != hash)
        return {};

    entry.eval = scoreFromTT(entry.eval, ply);
    ttHit = true;
    return entry;
}

// Saves an entry into the transposition table.
void ttSave(U64 hash, Depth depth, Score eval, EntryFlag flag, Move bestMove, Ply ply) {
    TTEntry *entry = getEntry(hash);

    if (entry->hash != hash || bestMove.isOk()) {
        entry->hashMove = bestMove;
    }

    if (entry->hash != hash || flag == TT_EXACT || entry->depth <= depth + 4) {
        entry->hash = hash;
        entry->depth = depth;
        entry->eval = scoreToTT(eval, ply);
        entry->flag = flag;
    }
}

// Returns the fullness of the transposition table
int getTTFull() {
    int cnt = 0;
    for (int i = 0; i < 1000; i++) {
        if (tt.table[i].hash != 0) cnt++;
    }
    return cnt;
}

// Returns the hash move corresponding to the Zobrist hash.
Move getHashMove(U64 hash) {
    TTEntry *entry = getEntry(hash);
    if (entry->hash == hash)
        return entry->hashMove;
    return MOVE_NULL;
}

// Prefetches a transposition table entry.
void ttPrefetch(U64 hash) {
    __builtin_prefetch(&tt.table[hash & tt.mask], 0, 1);
}