#ifndef BLACKCORE_THREADS_H
#define BLACKCORE_THREADS_H

#include "move.h"

#include <cstring>

struct ThreadData {

    int threadId;

    U64 nodes = 0;
    Depth selectiveDepth = 0;

    bool uciMode = false;

    Move pvArray[MAX_PLY + 1][MAX_PLY + 1];
    int pvLength[MAX_PLY + 1];

    inline void clear() {
        selectiveDepth = 0;
        uciMode = false;

        std::memset(pvArray, 0, sizeof(pvArray));
        std::memset(pvLength, 0, sizeof(pvLength));
    }

    inline void reset() {
        nodes = 0;

        clear();
    }
};

#endif//BLACKCORE_THREADS_H
