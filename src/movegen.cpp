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

#include "movegen.h"

template<Color color>
Move *generatePawnMoves(const Position &pos, Move *moves) {
    constexpr Color enemyColor = EnemyColor<color>();

    constexpr Direction UP = color == WHITE ? NORTH : -NORTH;
    constexpr Direction UP_LEFT = color == WHITE ? NORTH_WEST : -NORTH_WEST;
    constexpr Direction UP_RIGHT = color == WHITE ? NORTH_EAST : -NORTH_EAST;
    constexpr Direction DOWN = -UP;
    constexpr Direction DOWN_LEFT = -UP_RIGHT;
    constexpr Direction DOWN_RIGHT = -UP_LEFT;

    constexpr Bitboard doublePushRank = (color == WHITE ? rank3 : rank6);
    constexpr Bitboard beforePromoRank = (color == WHITE ? rank7 : rank2);
    constexpr Bitboard preventPush = ~beforePromoRank;

    Bitboard empty = pos.empty();
    Bitboard enemy = pos.enemy<color>();


    Bitboard pawns = pos.pieces<color, PAWN>();

    Bitboard singlePush = step<UP>(pawns & preventPush) & empty;
    Bitboard doublePush = step<UP>(singlePush & doublePushRank) & empty;

    Bitboard rightCapture = step<UP_RIGHT>(pawns) & enemy;
    Bitboard leftCapture = step<UP_LEFT>(pawns) & enemy;

    Bitboard pawnsBeforePromo = beforePromoRank & pawns;

    while (singlePush) {
        Square to = singlePush.popLsb();
        *moves++ = Move(to + DOWN, to, 0);
    }

    while (doublePush) {
        Square to = doublePush.popLsb();
        *moves++ = Move(to + (2 * DOWN), to, SPECIAL2_FLAG);
    }

    while (leftCapture) {
        Square to = leftCapture.popLsb();
        *moves++ = Move(to + DOWN_RIGHT, to, CAPTURE_FLAG, pos.pieceAt(to));
    }

    while (rightCapture) {
        Square to = rightCapture.popLsb();
        *moves++ = Move(to + DOWN_LEFT, to, CAPTURE_FLAG, pos.pieceAt(to));
    }

    if (pos.getEpSquare() != NULL_SQUARE) {
        Bitboard epPawns = pawnMask(pos.getEpSquare(), enemyColor) & pawns;

        while (epPawns) {
            *moves++ = Move(epPawns.popLsb(), pos.getEpSquare(), CAPTURE_FLAG | SPECIAL2_FLAG,
                            {PAWN, enemyColor});
        }
    }

    return moves;
}

template<Color color>
Move *generateMoves(const Position &pos, Move *moves) {
    Bitboard friendlyPieces = pos.friendly<color>();
    Bitboard enemyOrEmpty = pos.enemyOrEmpty<color>();
    Bitboard occupied = pos.occupied();

    moves = generatePawnMoves<color>(pos, moves);

    Bitboard temp = friendlyPieces & ~pos.pieces<color, PAWN>();
    while (temp) {
        Square from = temp.popLsb();
        PieceType type = pos.pieceAt(from).type;
        Bitboard attacks = pieceAttacks(type, from, occupied);
        attacks &= enemyOrEmpty;

        while (attacks) {
            Square to = attacks.popLsb();
            Piece piece = pos.pieceAt(to);

            if (piece.isNull()) {
                *moves++ = Move(from, to, 0);
            } else {
                *moves++ = Move(from, to, CAPTURE_FLAG, piece);
            }

        }
    }

    return moves;
}

Move *generateMoves(const Position &pos, Move *moves) {

    if (pos.getSideToMove() == WHITE) {
        return generateMoves<WHITE>(pos, moves);
    } else {
        return generateMoves<BLACK>(pos, moves);
    }
}
