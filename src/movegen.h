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

#ifndef BLACKCORE_MOVEGEN_H
#define BLACKCORE_MOVEGEN_H

#include "position.h"
#include "move.h"

template<Color color>
inline Bitboard getAttackers(const Position &pos, Square square) {
    Bitboard occupied = pos.occupied();
    Bitboard enemy = pos.enemy<color>();
    return ((pawnMask(square, color) & pos.pieces<PAWN>()) |
            (pieceAttacks<KNIGHT>(square, occupied) & pos.pieces<KNIGHT>()) |
            (pieceAttacks<BISHOP>(square, occupied) & pos.pieces<BISHOP>()) |
            (pieceAttacks<ROOK>(square, occupied) & pos.pieces<ROOK>()) |
            (pieceAttacks<QUEEN>(square, occupied) & pos.pieces<QUEEN>())) & enemy;
}

Move *generateMoves(const Position &pos, Move *moves);

#endif //BLACKCORE_MOVEGEN_H
