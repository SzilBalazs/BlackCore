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

// Generates all the promotions of a move.
inline Move *makePromo(Move *moves, Square from, Square to) {
    *moves++ = Move(from, to, PROMO_KNIGHT);
    *moves++ = Move(from, to, PROMO_BISHOP);
    *moves++ = Move(from, to, PROMO_ROOK);
    *moves++ = Move(from, to, PROMO_QUEEN);
    return moves;
}

// Generates all the promotion captures of a move.
inline Move *makePromoCapture(Move *moves, Square from, Square to) {
    *moves++ = Move(from, to, PROMO_CAPTURE_KNIGHT);
    *moves++ = Move(from, to, PROMO_CAPTURE_BISHOP);
    *moves++ = Move(from, to, PROMO_CAPTURE_ROOK);
    *moves++ = Move(from, to, PROMO_CAPTURE_QUEEN);
    return moves;
}

// Returns all the attacked squares by a side.
template<Color color>
inline Bitboard getAttackedSquares(const Position &pos, Bitboard occupied) {

    constexpr Direction UP_LEFT = color == WHITE ? NORTH_WEST : -NORTH_WEST;
    constexpr Direction UP_RIGHT = color == WHITE ? NORTH_EAST : -NORTH_EAST;

    Bitboard pawns = pos.pieces<color, PAWN>();
    Bitboard pieces = pos.friendly<color>() & ~pawns;
    Bitboard result = step<UP_LEFT>(pawns) | step<UP_RIGHT>(pawns);

    while (pieces) {
        Square from = pieces.popLsb();
        result |= pieceAttacks<color>(pos.pieceAt(from).type, from, occupied);
    }

    return result;
}

// Generates all legal moves of the pieces.
template<bool capturesOnly, bool pinHV, bool pinDA>
inline Move *generateMovesFromPieces(const Position &pos, Move *moves, Bitboard pieces, Bitboard specialMask,
                                     Bitboard occupied, Bitboard empty, Bitboard enemy) {

    while (pieces) {
        Square from = pieces.popLsb();
        PieceType type = pos.pieceAt(from).type;
        Bitboard attacks = pieceAttacks(type, from, occupied) & specialMask;
        if constexpr (pinHV)
            attacks &= rookMasks[from];
        if constexpr (pinDA)
            attacks &= bishopMasks[from];

        if constexpr (!capturesOnly) {
            Bitboard quiets = attacks & empty;
            while (quiets) {
                *moves++ = Move(from, quiets.popLsb());
            }
        }

        Bitboard captures = attacks & enemy;
        while (captures) {
            Square to = captures.popLsb();
            *moves++ = Move(from, to, CAPTURE);
        }
    }

    return moves;
}

// Generates all the legal pawn moves.
template<Color color, bool capturesOnly>
Move *generatePawnMoves(const Position &pos, Move *moves, Square king, Bitboard checkMask,
                        Bitboard moveH, Bitboard moveV, Bitboard moveD, Bitboard moveA) {
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

    Square epSquare = pos.getEpSquare();

    Bitboard empty = pos.empty();
    Bitboard enemy = pos.enemy<color>();

    Bitboard pawns = pos.pieces<color, PAWN>();
    Bitboard pawnsBeforePromo = beforePromoRank & pawns;
    pawns &= notBeforePromo;

    if constexpr (!capturesOnly) {
        Bitboard singlePush = step<UP>(pawns & moveH) & empty;
        Bitboard doublePush = step<UP>(singlePush & doublePushRank) & empty;

        singlePush &= checkMask;
        doublePush &= checkMask;

        while (singlePush) {
            Square to = singlePush.popLsb();
            *moves++ = Move(to + DOWN, to);
        }

        while (doublePush) {
            Square to = doublePush.popLsb();
            *moves++ = Move(to + (2 * DOWN), to, DOUBLE_PAWN_PUSH);
        }
    }

    Bitboard rightCapture = step<UP_RIGHT>(pawns & moveD) & enemy & checkMask;
    Bitboard leftCapture = step<UP_LEFT>(pawns & moveA) & enemy & checkMask;

    while (leftCapture) {
        Square to = leftCapture.popLsb();
        *moves++ = Move(to + DOWN_RIGHT, to, CAPTURE);
    }

    while (rightCapture) {
        Square to = rightCapture.popLsb();
        *moves++ = Move(to + DOWN_LEFT, to, CAPTURE);
    }

    if (pawnsBeforePromo) {

        if constexpr (!capturesOnly) {
            Bitboard upPromo = step<UP>(pawnsBeforePromo & moveH) & empty & checkMask;
            while (upPromo) {
                Square to = upPromo.popLsb();
                moves = makePromo(moves, to + DOWN, to);
            }
        }

        Bitboard rightPromo = step<UP_RIGHT>(pawnsBeforePromo & moveD) & enemy & checkMask;
        Bitboard leftPromo = step<UP_LEFT>(pawnsBeforePromo & moveA) & enemy & checkMask;
        while (rightPromo) {
            Square to = rightPromo.popLsb();
            moves = makePromoCapture(moves, to + DOWN_LEFT, to);
        }

        while (leftPromo) {
            Square to = leftPromo.popLsb();
            moves = makePromoCapture(moves, to + DOWN_RIGHT, to);
        }
    }

    if ((epSquare != NULL_SQUARE) && (pawnMasks[pos.getEpSquare()][enemyColor] & pawns) &&
        checkMask.get(epSquare + DOWN)) {
        Bitboard occ = pos.occupied();
        bool rightEp = (step<UP_RIGHT>(pawns & moveD)).get(epSquare);
        bool leftEp = (step<UP_LEFT>(pawns & moveA)).get(epSquare);

        if (rightEp) {
            Square attackingPawn = epSquare + DOWN_LEFT;
            Square attackedPawn = epSquare + DOWN;

            occ.clear(attackingPawn);
            occ.clear(attackedPawn);

            Bitboard rookAttack = rookAttacks(attackedPawn, occ);
            Bitboard bishopAttack = bishopAttacks(attackedPawn, occ);

            Bitboard rankAttack = rankMasks[attackedPawn] & rookAttack;
            Bitboard diagAttack = diagonalMasks[attackedPawn] & bishopAttack;
            Bitboard aDiagAttack = antiDiagonalMasks[attackedPawn] & bishopAttack;

            Bitboard seenRankSliders = (pos.pieces<enemyColor, QUEEN>() | pos.pieces<enemyColor, ROOK>()) & rankAttack;
            Bitboard seenDiagSliders =
                    (pos.pieces<enemyColor, QUEEN>() | pos.pieces<enemyColor, BISHOP>()) & diagAttack;
            Bitboard seenADiagSliders =
                    (pos.pieces<enemyColor, QUEEN>() | pos.pieces<enemyColor, BISHOP>()) & aDiagAttack;

            bool pinRank = rankAttack.get(king) && seenRankSliders;
            bool pinDiag = diagAttack.get(king) && seenDiagSliders;
            bool pinADiag = aDiagAttack.get(king) && seenADiagSliders;

            if (!(pinRank || pinDiag || pinADiag))
                *moves++ = Move(attackingPawn, epSquare, EP_CAPTURE);

            occ.set(attackingPawn);
            occ.set(attackedPawn);
        }

        if (leftEp) {
            Square attackingPawn = epSquare + DOWN_RIGHT;
            Square attackedPawn = epSquare + DOWN;

            occ.clear(attackingPawn);
            occ.clear(attackedPawn);

            Bitboard rookAttack = rookAttacks(attackedPawn, occ);
            Bitboard bishopAttack = bishopAttacks(attackedPawn, occ);

            Bitboard rankAttack = rankMasks[attackedPawn] & rookAttack;
            Bitboard diagAttack = diagonalMasks[attackedPawn] & bishopAttack;
            Bitboard aDiagAttack = antiDiagonalMasks[attackedPawn] & bishopAttack;

            Bitboard seenRankSliders = (pos.pieces<enemyColor, QUEEN>() | pos.pieces<enemyColor, ROOK>()) & rankAttack;
            Bitboard seenDiagSliders =
                    (pos.pieces<enemyColor, QUEEN>() | pos.pieces<enemyColor, BISHOP>()) & diagAttack;
            Bitboard seenADiagSliders =
                    (pos.pieces<enemyColor, QUEEN>() | pos.pieces<enemyColor, BISHOP>()) & aDiagAttack;

            bool pinRank = rankAttack.get(king) && seenRankSliders;
            bool pinDiag = diagAttack.get(king) && seenDiagSliders;
            bool pinADiag = aDiagAttack.get(king) && seenADiagSliders;

            if (!(pinRank || pinDiag || pinADiag))
                *moves++ = Move(attackingPawn, epSquare, EP_CAPTURE);

            occ.set(attackingPawn);
            occ.set(attackedPawn);
        }
    }

    return moves;
}

// Generates all the legal king moves.
template<bool capturesOnly>
inline Move *generateKingMoves(const Position &pos, Move *moves, Square king,
                               Bitboard safeSquares, Bitboard empty, Bitboard enemy) {

    Bitboard kingTarget = kingMasks[king] & safeSquares;

    if constexpr (!capturesOnly) {
        Bitboard kingQuiets = kingTarget & empty;
        while (kingQuiets) {
            *moves++ = Move(king, kingQuiets.popLsb());
        }
    }

    Bitboard kingCaptures = kingTarget & enemy;
    while (kingCaptures) {
        Square to = kingCaptures.popLsb();
        *moves++ = Move(king, to, CAPTURE);
    }

    return moves;
}

// Returns the "to" squares which evades check.
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

// Generates all the legal slider and knight moves using the generateMovesFromPieces utility.
template<bool capturesOnly>
inline Move *generateSliderAndJumpMoves(const Position &pos, Move *moves, Bitboard pieces,
                                        Bitboard occupied, Bitboard empty, Bitboard enemy, Bitboard checkMask,
                                        Bitboard pinHV, Bitboard pinDA) {
    Bitboard pinnedHV = pinHV & pieces;
    Bitboard pinnedDA = pinDA & pieces;
    pieces &= ~(pinnedHV | pinnedDA);

    moves = generateMovesFromPieces<capturesOnly, false, false>(pos, moves, pieces, checkMask, occupied, empty, enemy);

    moves = generateMovesFromPieces<capturesOnly, true, false>(pos, moves, pinnedHV, checkMask & pinHV, occupied, empty,
                                                               enemy);

    moves = generateMovesFromPieces<capturesOnly, false, true>(pos, moves, pinnedDA, checkMask & pinDA, occupied, empty,
                                                               enemy);

    return moves;
}

// Generates all the legal moves in a position.
template<Color color, bool capturesOnly>
Move *generateMoves(const Position &pos, Move *moves) {
    constexpr Color enemyColor = EnemyColor<color>();

    Square king = pos.pieces<color, KING>().lsb();
    assert(king != NULL_SQUARE);

    Bitboard friendlyPieces = pos.friendly<color>();
    Bitboard empty = pos.empty();
    Bitboard enemy = pos.enemy<color>();
    Bitboard occupied = pos.occupied();
    Bitboard checkers = getAttackers<color>(pos, king);

    occupied.clear(king);
    Bitboard safeSquares = ~getAttackedSquares<enemyColor>(pos, occupied);
    occupied.set(king);

    // Generating checkMask
    Bitboard checkMask = generateCheckMask(pos, king, checkers);

    // Generating king moves
    moves = generateKingMoves<capturesOnly>(pos, moves, king, safeSquares, empty, enemy);

    // If we are in a double check, only king moves are legal
    if (checkMask == 0)
        return moves;

    // Generating pinMasks
    Bitboard seenSquares = pieceAttacks<QUEEN>(king, occupied);
    Bitboard possiblePins = seenSquares & friendlyPieces;

    occupied ^= possiblePins;

    Bitboard possiblePinners = (pieceAttacks<QUEEN>(king, occupied) ^ seenSquares) & enemy;
    Bitboard pinners = ((pieceAttacks<ROOK>(king, occupied) & pos.pieces<ROOK>()) |
                        (pieceAttacks<BISHOP>(king, occupied) & pos.pieces<BISHOP>()) |
                        (pieceAttacks<QUEEN>(king, occupied) & pos.pieces<QUEEN>())) &
                       possiblePinners;

    Bitboard pinH, pinV, pinD, pinA, pinHV, pinDA, moveH, moveV, moveD, moveA;

    while (pinners) {
        Square pinner = pinners.popLsb();
        LineType type = lineType[king][pinner];
        switch (type) {
            case HORIZONTAL:
                pinH |= commonRay[king][pinner] | pinner;
                break;
            case VERTICAL:
                pinV |= commonRay[king][pinner] | pinner;
                break;
            case DIAGONAL:
                pinD |= commonRay[king][pinner] | pinner;
                break;
            case ANTI_DIAGONAL:
                pinA |= commonRay[king][pinner] | pinner;
                break;
        }
    }

    pinHV = pinH | pinV;
    pinDA = pinD | pinA;

    pinH &= friendlyPieces;
    pinV &= friendlyPieces;
    pinD &= friendlyPieces;
    pinA &= friendlyPieces;

    moveH = ~(pinV | pinD | pinA);
    moveV = ~(pinH | pinD | pinA);
    moveD = ~(pinH | pinV | pinA);
    moveA = ~(pinH | pinV | pinD);

    occupied ^= possiblePins;

    // Generating pawn moves
    moves = generatePawnMoves<color, capturesOnly>(pos, moves, king, checkMask, moveH, moveV, moveD, moveA);

    // Generating knight and slider moves
    Bitboard sliderAndJumperPieces = friendlyPieces & ~pos.pieces<PAWN>();
    sliderAndJumperPieces.clear(king);

    moves = generateSliderAndJumpMoves<capturesOnly>(pos, moves, sliderAndJumperPieces, occupied, empty, enemy,
                                                     checkMask, pinHV, pinDA);

    // Generating castling moves
    if constexpr (!capturesOnly) {
        if constexpr (color == WHITE) {
            if (pos.getCastleRight(WK_MASK) &&
                (safeSquares & WK_CASTLE_SAFE) == WK_CASTLE_SAFE && (empty & WK_CASTLE_EMPTY) == WK_CASTLE_EMPTY) {

                *moves++ = Move(E1, G1, KING_CASTLE);
            }

            if (pos.getCastleRight(WQ_MASK) &&
                (safeSquares & WQ_CASTLE_SAFE) == WQ_CASTLE_SAFE && (empty & WQ_CASTLE_EMPTY) == WQ_CASTLE_EMPTY) {

                *moves++ = Move(E1, C1, QUEEN_CASTLE);
            }
        } else {
            if (pos.getCastleRight(BK_MASK) &&
                (safeSquares & BK_CASTLE_SAFE) == BK_CASTLE_SAFE && (empty & BK_CASTLE_EMPTY) == BK_CASTLE_EMPTY) {

                *moves++ = Move(E8, G8, KING_CASTLE);
            }

            if (pos.getCastleRight(BQ_MASK) &&
                (safeSquares & BQ_CASTLE_SAFE) == BQ_CASTLE_SAFE && (empty & BQ_CASTLE_EMPTY) == BQ_CASTLE_EMPTY) {

                *moves++ = Move(E8, C8, QUEEN_CASTLE);
            }
        }
    }

    return moves;
}

// Wrapper around the stm template.
template<bool capturesOnly>
Move *generateMoves(const Position &pos, Move *moves) {
    if (pos.getSideToMove() == WHITE) {
        return generateMoves<WHITE, capturesOnly>(pos, moves);
    } else {
        return generateMoves<BLACK, capturesOnly>(pos, moves);
    }
}

// Wrapper around captures only template.
Move *generateMoves(const Position &pos, Move *moves, bool capturesOnly) {
    if (capturesOnly) {
        return generateMoves<true>(pos, moves);
    } else {
        return generateMoves<false>(pos, moves);
    }
}