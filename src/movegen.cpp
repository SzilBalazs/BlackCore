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

Move *makePromo(Move *moves, Square from, Square to) {
    // Knight
    *moves++ = Move(from, to, PROMO_FLAG);

    // Bishop
    *moves++ = Move(from, to, PROMO_FLAG | SPECIAL2_FLAG);

    // Rook
    *moves++ = Move(from, to, PROMO_FLAG | SPECIAL1_FLAG);

    // Queen
    *moves++ = Move(from, to, PROMO_FLAG | SPECIAL1_FLAG | SPECIAL2_FLAG);
    return moves;
}

template<Color color>
inline Bitboard getCheckers(const Position &pos, Square king) {
    Bitboard occupied = pos.occupied();
    Bitboard enemy = pos.enemy<color>();
    return ((pawnMask(king, color) & pos.pieces<PAWN>()) |
            (pieceAttacks<KNIGHT>(king, occupied) & pos.pieces<KNIGHT>()) |
            (pieceAttacks<BISHOP>(king, occupied) & pos.pieces<BISHOP>()) |
            (pieceAttacks<ROOK>(king, occupied) & pos.pieces<ROOK>()) |
            (pieceAttacks<QUEEN>(king, occupied) & pos.pieces<QUEEN>())) & enemy;
}

template<Color color>
inline Bitboard getAttackedSquares(const Position &pos, Bitboard occupied) {
    Bitboard pieces = pos.friendly<color>();
    Bitboard result = 0;

    while (pieces) {
        Square from = pieces.popLsb();
        result |= pieceAttacks<color>(pos.pieceAt(from).type, from, occupied);
    }

    return result;
}

template<Color color>
Move *generatePawnMoves(const Position &pos, Move *moves, Bitboard checkMask) {
    constexpr Color enemyColor = EnemyColor<color>();

    constexpr Direction UP = color == WHITE ? NORTH : -NORTH;
    constexpr Direction UP_LEFT = color == WHITE ? NORTH_WEST : -NORTH_WEST;
    constexpr Direction UP_RIGHT = color == WHITE ? NORTH_EAST : -NORTH_EAST;
    constexpr Direction DOWN = -UP;
    constexpr Direction DOWN_LEFT = -UP_RIGHT;
    constexpr Direction DOWN_RIGHT = -UP_LEFT;

    constexpr Bitboard doublePushRank = (color == WHITE ? rank3 : rank6);
    constexpr Bitboard beforePromoRank = (color == WHITE ? rank7 : rank2);
    constexpr Bitboard notBeforePromo = ~beforePromoRank;

    Bitboard empty = pos.empty();
    Bitboard enemy = pos.enemy<color>();

    Bitboard pawns = pos.pieces<color, PAWN>();
    Bitboard pawnsBeforePromo = beforePromoRank & pawns;
    pawns &= notBeforePromo;

    Bitboard singlePush = step<UP>(pawns) & empty;
    Bitboard doublePush = step<UP>(singlePush & doublePushRank) & empty;

    singlePush &= checkMask;
    doublePush &= checkMask;

    Bitboard rightCapture = step<UP_RIGHT>(pawns) & enemy & checkMask;
    Bitboard leftCapture = step<UP_LEFT>(pawns) & enemy & checkMask;

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

    if (pawnsBeforePromo) {
        Bitboard upPromo = step<UP>(pawnsBeforePromo) & empty & checkMask;
        Bitboard rightPromo = step<UP_RIGHT>(pawnsBeforePromo) & enemy & checkMask;
        Bitboard leftPromo = step<UP_LEFT>(pawnsBeforePromo) & enemy & checkMask;

        while (upPromo) {
            Square to = upPromo.popLsb();
            moves = makePromo(moves, to + DOWN, to);
        }

        while (rightPromo) {
            Square to = rightPromo.popLsb();
            moves = makePromo(moves, to + DOWN_LEFT, to);
        }

        while (leftPromo) {
            Square to = leftPromo.popLsb();
            moves = makePromo(moves, to + DOWN_RIGHT, to);
        }
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

inline Move *generateKingMoves(const Position &pos, Move *moves, Square king,
                               Bitboard safeSquares, Bitboard empty, Bitboard enemy) {

    Bitboard kingTarget = kingMask(king) & safeSquares;
    Bitboard kingQuiets = kingTarget & empty;
    Bitboard kingCaptures = kingTarget & enemy;

    while (kingQuiets) {
        *moves++ = Move(king, kingQuiets.popLsb(), 0);
    }

    while (kingCaptures) {
        Square to = kingCaptures.popLsb();
        *moves++ = Move(king, to, CAPTURE_FLAG, pos.pieceAt(to));
    }

    return moves;
}

inline Bitboard generateCheckMask(const Position &pos, Square king, Bitboard checkers) {
    unsigned int checks = checkers.popCount();
    if (checks == 0) {
        return 0xffffffffffffffffULL;
    } else if (checks == 1) {
        Square checker = checkers.lsb();
        PieceType type = pos.pieceAt(checker).type;
        if (type == ROOK || type == BISHOP || type == QUEEN) {
            return checkers | commonRay[king][checker];
        } else {
            return checkers;
        }
    } else {
        return 0;
    }
}

inline Move *
generateSliderAndJumpMoves(const Position &pos, Move *moves, Bitboard pieces, Bitboard occupied, Bitboard empty,
                           Bitboard enemy, Bitboard checkMask) {

    while (pieces) {
        Square from = pieces.popLsb();
        PieceType type = pos.pieceAt(from).type;
        Bitboard attacks = pieceAttacks(type, from, occupied) & checkMask;
        Bitboard quiets = attacks & empty;
        Bitboard captures = attacks & enemy;

        while (quiets) {
            *moves++ = Move(from, quiets.popLsb(), 0);
        }

        while (captures) {
            Square to = captures.popLsb();
            *moves++ = Move(from, to, CAPTURE_FLAG, pos.pieceAt(to));
        }
    }

    return moves;
}

template<Color color>
Move *generateMoves(const Position &pos, Move *moves) {
    constexpr Color enemyColor = EnemyColor<color>();

    Square king = pos.pieces<color, KING>().lsb();
    assert(king != NULL_SQUARE);

    Bitboard friendlyPieces = pos.friendly<color>();
    Bitboard empty = pos.empty();
    Bitboard enemy = pos.enemy<color>();
    Bitboard occupied = pos.occupied();
    Bitboard checkers = getCheckers<color>(pos, king);

    occupied.clear(king);
    Bitboard safeSquares = ~getAttackedSquares<enemyColor>(pos, occupied);
    occupied.set(king);

    // Generating checkMask
    Bitboard checkMask = generateCheckMask(pos, king, checkers);

    // Generating king moves
    moves = generateKingMoves(pos, moves, king, safeSquares, empty, enemy);

    if (checkMask == 0)
        return moves;

    // Generating pawn moves
    moves = generatePawnMoves<color>(pos, moves, checkMask);

    // Generating knight and slider moves
    Bitboard sliderAndJumperPieces = friendlyPieces & ~pos.pieces<color, PAWN>();
    sliderAndJumperPieces.clear(king);

    moves = generateSliderAndJumpMoves(pos, moves, sliderAndJumperPieces, occupied, empty, enemy, checkMask);

    return moves;
}

Move *generateMoves(const Position &pos, Move *moves) {

    if (pos.getSideToMove() == WHITE) {
        return generateMoves<WHITE>(pos, moves);
    } else {
        return generateMoves<BLACK>(pos, moves);
    }
}
