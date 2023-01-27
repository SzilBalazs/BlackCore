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

#ifndef BLACKCORE_MOVEGEN_H
#define BLACKCORE_MOVEGEN_H

#include "move.h"
#include "position.h"
#include "threads.h"

// Returns a bitboard of all the squares attacking a given square for the given color
template<Color color>
inline Bitboard getAttackers(const Position &pos, Square square) {
    Bitboard occupied = pos.occupied();
    Bitboard enemy = pos.enemy<color>();
    return ((pawnMasks[square][color] & pos.pieces<PAWN>()) |
            (pieceAttacks<KNIGHT>(square, occupied) & pos.pieces<KNIGHT>()) |
            (pieceAttacks<BISHOP>(square, occupied) & pos.pieces<BISHOP>()) |
            (pieceAttacks<ROOK>(square, occupied) & pos.pieces<ROOK>()) |
            (pieceAttacks<QUEEN>(square, occupied) & pos.pieces<QUEEN>())) &
           enemy;
}

// Returns a bitboard of all the squares attacking a given square for the side to move
inline Bitboard getAttackers(const Position &pos, Square square) {
    if (pos.getSideToMove() == WHITE)
        return getAttackers<WHITE>(pos, square);
    else
        return getAttackers<BLACK>(pos, square);
}

Move *generateMoves(const Position &pos, Move *moves, bool capturesOnly);

// Stores and orders legal moves in a position.
template<bool capturesOnly, bool rootNode>
struct MoveList {
    Move moves[200];
    Move *movesEnd;
    unsigned int index;

    Score scores[200];

    unsigned int count;

    // Constructor that generates and scores legal moves.
    MoveList(const Position &pos, ThreadData &td, SearchStack *stack) {
        movesEnd = generateMoves(pos, moves, capturesOnly);
        index = 0;
        count = movesEnd - moves;

        for (unsigned int i = 0; i < count; i++) {
            scores[i] = rootNode ? td.scoreRootNode(moves[i]) : td.scoreMove(stack, moves[i]);
        }
    }

    // Returns true if there are no more moves left.
    inline bool empty() const {
        return index == count;
    }

    // Sorts and returns the next best scored move.
    inline Move nextMove() {
        unsigned int best = index;
        for (unsigned int i = index; i < count; i++) {
            if (scores[i] > scores[best]) {
                best = i;
            }
        }
        std::swap(moves[index], moves[best]);
        std::swap(scores[index], scores[best]);

        return moves[index++];
    }
};

#endif //BLACKCORE_MOVEGEN_H
