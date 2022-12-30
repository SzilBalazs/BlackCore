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
void ttResize(unsigned int MBSize) {

    if (tt.bucketCount)
        ttFree();

    unsigned int i = 10;
    while ((1ULL << i) <= MBSize * 1024 * 1024 / sizeof(TTEntry))
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
TTEntry ttProbe(U64 hash, bool &ttHit) {
    TTEntry *entry = getEntry(hash);

    if (entry->hash != hash || std::abs(entry->eval) > MATE_VALUE - 100)
        return {};

    ttHit = true;
    return *entry;
}

// Saves an entry into the transposition table.
void ttSave(U64 hash, Depth depth, Score eval, EntryFlag flag, Move bestMove) {
    TTEntry *entry = getEntry(hash);

    if (entry->hash != hash || flag == TT_EXACT || entry->depth * 2 / 3 <= depth) {
        entry->hash = hash;
        entry->depth = depth;
        entry->eval = eval;
        entry->flag = flag;
        entry->hashMove = bestMove;
    }
}

// Returns the hash move corresponding to the Zobrist hash.
Move getHashMove(U64 hash) {
    TTEntry *entry = getEntry(hash);
    if (entry->hash == hash)
        return entry->hashMove;
    return {};
}

// Prefetches a transposition table entry.
void ttPrefetch(U64 hash) {
    __builtin_prefetch(&tt.table[hash & tt.mask], 0, 1);
}