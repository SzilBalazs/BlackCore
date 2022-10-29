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

#include <cstring>
#include "tt.h"

#ifdef __linux__

#include <sys/mman.h>

#endif

TTable tt;
uint16_t globalAge = 0;

TTBucket *getBucket(U64 hash) {
    return tt.table + (hash & tt.mask);
}

void ttClear() {
    std::memset(tt.table, 0, tt.bucketCount * sizeof(TTBucket));
    globalAge = 0;
}

void ttFree() {
    free(tt.table);
}

void ttResize(unsigned int MBSize) {

    if (tt.bucketCount)
        ttFree();

    unsigned int i = 10;
    while ((1ULL << i) <= MBSize * 1024 * 1024 / sizeof(TTBucket)) i++;

    tt.bucketCount = (1ULL << (i - 1));
    tt.mask = tt.bucketCount - 1ULL;


#ifdef __linux__
    // Allocate memory with 1MB alignment
    tt.table = static_cast<TTBucket *>(aligned_alloc((1ULL << 20), tt.bucketCount * sizeof(TTBucket)));

    // For reference see https://man7.org/linux/man-pages/man2/madvise.2.html on MADV HUGEPAGE
    madvise(tt.table, tt.bucketCount * sizeof(TTBucket), MADV_HUGEPAGE);
#else
    tt.table = (TTBucket*)malloc(tt.bucketCount * sizeof(TTBucket));
#endif

    ttClear();

}

TTEntry *ttProbe(U64 hash, bool &ttHit, Depth depth, Score alpha, Score beta) {
    TTBucket *bucket = getBucket(hash);
    TTEntry *entry;
    if (bucket->entryA.hash == hash) {
        entry = &bucket->entryA;
        entry->age = globalAge;
    } else if (bucket->entryB.hash == hash) {
        entry = &bucket->entryB;
    } else {
        return nullptr;
    }

    if (std::abs(entry->eval) > MATE_VALUE - 100) return nullptr;

    ttHit = true;
    return entry;
}

void ttSave(U64 hash, Depth depth, Score eval, EntryFlag flag, Move bestMove) {
    TTBucket *bucket = getBucket(hash);
    TTEntry *entry;

    if (bucket->entryA.hash == hash || bucket->entryA.depth * 2 / 3 <= depth ||
        globalAge - bucket->entryA.age >= 3) {
        entry = &bucket->entryA;
    } else {
        entry = &bucket->entryB;
    }

    if (entry->hash != hash || flag == EXACT || entry->depth * 2 / 3 <= depth) {
        entry->hash = hash;
        entry->depth = depth;
        entry->eval = eval;
        entry->flag = flag;
        entry->hashMove = bestMove;
        entry->age = globalAge;
    }

}

Move getHashMove(U64 hash) {
    TTBucket *bucket = getBucket(hash);
    if (bucket->entryA.hash == hash) return bucket->entryA.hashMove;
    else if (bucket->entryB.hash == hash) return bucket->entryB.hashMove;
    return {};
}

void ttPrefetch(U64 hash) {
    __builtin_prefetch(&tt.table[hash & tt.mask], 0, 1);
}