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

#include "movegen.h"

// Generates all promotion moves for a pawn from 'from' to 'to'
// and adds them to the move list 'moves'
inline Move *makePromo(Move *moves, Square from, Square to) {
    *moves++ = Move(from, to, PROMO_KNIGHT);
    *moves++ = Move(from, to, PROMO_BISHOP);
    *moves++ = Move(from, to, PROMO_ROOK);
    *moves++ = Move(from, to, PROMO_QUEEN);
    return moves;
}

// Generates all promotion captures for a pawn from 'from' to 'to'
// and adds them to the move list 'moves'
inline Move *makePromoCapture(Move *moves, Square from, Square to) {
    *moves++ = Move(from, to, PROMO_CAPTURE_KNIGHT);
    *moves++ = Move(from, to, PROMO_CAPTURE_BISHOP);
    *moves++ = Move(from, to, PROMO_CAPTURE_ROOK);
    *moves++ = Move(from, to, PROMO_CAPTURE_QUEEN);
    return moves;
}

// Returns a bitboard of all the squares attacked by a given color
// 'pos' is the position, 'occupied' is a bitboard of all the occupied squares
template<Color color>
inline Bitboard getAttackedSquares(const Position &pos, Bitboard occupied) {

    constexpr Direction UP_LEFT = color == WHITE ? NORTH_WEST : -NORTH_WEST;
    constexpr Direction UP_RIGHT = color == WHITE ? NORTH_EAST : -NORTH_EAST;

    // Get the pawns of the given color and all other pieces
    Bitboard pawns = pos.pieces<color, PAWN>();
    Bitboard pieces = pos.friendly<color>() & ~pawns;

    // Initially add the attacks of the pawns
    Bitboard result = step<UP_LEFT>(pawns) | step<UP_RIGHT>(pawns);

    // Iterate through all other pieces and add their attacks
    while (pieces) {
        Square from = pieces.popLsb();
        result |= pieceAttacks<color>(pos.pieceAt(from).type, from, occupied);
    }

    return result;
}

// Generates all legal moves for the given pieces
// 'capturesOnly' - if true, only generates capture moves
// 'pinHV' - if true, pieces are pinned horizontally or vertically
// 'pinDA' - if true, pieces are pinned diagonally or anti-diagonally
// 'pieces' - bitboard of the pieces to generate moves for
// 'specialMask' - a bitboard indicating the squares to generate moves to
// 'occupied' - a bitboard of all occupied squares
// 'empty' - a bitboard of all empty squares
// 'enemy' - a bitboard of all enemy pieces
template<bool capturesOnly, bool pinHV, bool pinDA>
inline Move *generateMovesFromPieces(const Position &pos, Move *moves, Bitboard pieces, Bitboard specialMask,
                                     Bitboard occupied, Bitboard empty, Bitboard enemy) {

    // Iterate through the pieces
    while (pieces) {

        Square from = pieces.popLsb();
        PieceType type = pos.pieceAt(from).type;

        // Get the attacks of the piece and filter by the special mask
        Bitboard attacks = pieceAttacks(type, from, occupied) & specialMask;

        // Check if the piece is pinned horizontally or vertically
        if constexpr (pinHV)
            attacks &= rookMasks[from];

        // Check if the piece is pinned diagonally
        if constexpr (pinDA)
            attacks &= bishopMasks[from];

        // If we're not generating captures only, generate quiet moves
        if constexpr (!capturesOnly) {
            Bitboard quiets = attacks & empty;
            while (quiets) {
                *moves++ = Move(from, quiets.popLsb());
            }
        }

        // Generate captures
        Bitboard captures = attacks & enemy;
        while (captures) {
            Square to = captures.popLsb();
            *moves++ = Move(from, to, CAPTURE);
        }
    }

    return moves;
}

// Generates all legal pawn moves for all the 'color' pawns in 'pos' position and
// adds them to the 'moves' move list.
// 'capturesOnly' - if true, only generates capture moves
// 'king' - the square where the friendly king is
// 'checkMask' - a bitboard indicating to squares which evade check
// 'moveH' - a bitboard indicating pieces which can move horizontally
// 'moveV' - a bitboard indicating pieces which can move vertically
// 'moveD' - a bitboard indicating pieces which can move diagonally
// 'moveA' - a bitboard indicating pieces which can move anti-diagonally
template<Color color, bool capturesOnly>
Move *generatePawnMoves(const Position &pos, Move *moves, Square king, Bitboard checkMask,
                        Bitboard moveH, Bitboard moveV, Bitboard moveD, Bitboard moveA) {

    // Define enemy color
    constexpr Color enemyColor = EnemyColor<color>();

    // Define move directions
    constexpr Direction UP = color == WHITE ? NORTH : -NORTH;
    constexpr Direction UP_LEFT = color == WHITE ? NORTH_WEST : -NORTH_WEST;
    constexpr Direction UP_RIGHT = color == WHITE ? NORTH_EAST : -NORTH_EAST;
    constexpr Direction DOWN = -UP;
    constexpr Direction DOWN_LEFT = -UP_RIGHT;
    constexpr Direction DOWN_RIGHT = -UP_LEFT;

    // Define ranks for double pawn push and promotion
    constexpr Bitboard doublePushRank = (color == WHITE ? rank3 : rank6);
    constexpr Bitboard beforePromoRank = (color == WHITE ? rank7 : rank2);

    // Create a bitboard of squares not on the promotion rank
    constexpr Bitboard notBeforePromo = ~beforePromoRank;

    // Get the en passant square
    Square epSquare = pos.getEpSquare();

    // Get the empty and the enemy bitboards
    Bitboard empty = pos.empty();
    Bitboard enemy = pos.enemy<color>();

    // Get the bitboard of pawns to generate moves for
    Bitboard pawns = pos.pieces<color, PAWN>();

    // Get the bitboard of pawns before promotion
    Bitboard pawnsBeforePromo = beforePromoRank & pawns;
    pawns &= notBeforePromo;

    // Generate quiet moves
    if constexpr (!capturesOnly) {

        Bitboard singlePush = step<UP>(pawns & moveH) & empty;               // Generates single pawn pushes
        Bitboard doublePush = step<UP>(singlePush & doublePushRank) & empty; // Generates double pawn pushes

        // Filter out moves that are within the 'checkMask'
        singlePush &= checkMask;
        doublePush &= checkMask;

        // Iterate through single pawn pushes
        while (singlePush) {
            Square to = singlePush.popLsb();
            *moves++ = Move(to + DOWN, to);
        }

        // Iterate through double pawn pushes
        while (doublePush) {
            Square to = doublePush.popLsb();
            *moves++ = Move(to + (2 * DOWN), to, DOUBLE_PAWN_PUSH);
        }
    }

    // Filter out all the legal capture moves to the right
    Bitboard rightCapture = step<UP_RIGHT>(pawns & moveD) & enemy & checkMask;
    // Filter out all the legal capture moves to the left
    Bitboard leftCapture = step<UP_LEFT>(pawns & moveA) & enemy & checkMask;

    // Iterate through left captures
    while (leftCapture) {
        Square to = leftCapture.popLsb();
        *moves++ = Move(to + DOWN_RIGHT, to, CAPTURE);
    }

    // Iterate through right captures
    while (rightCapture) {
        Square to = rightCapture.popLsb();
        *moves++ = Move(to + DOWN_LEFT, to, CAPTURE);
    }

    // Check if there are any pawns that can be promoted
    if (pawnsBeforePromo) {

        // Generate quiet moves
        if constexpr (!capturesOnly) {
            // Filter out all the legal promotions upwards
            Bitboard upPromo = step<UP>(pawnsBeforePromo & moveH) & empty & checkMask;

            while (upPromo) {
                Square to = upPromo.popLsb();
                moves = makePromo(moves, to + DOWN, to);
            }
        }

        // Filter out all the legal sideways capture-promotions
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

    // Check if the epSquare is not empty, if there is an en-passantable pawn and if the epSquare is within the check mask
    if ((epSquare != NULL_SQUARE) && (pawnMasks[pos.getEpSquare()][enemyColor] & pawns) &&
        checkMask.get(epSquare + DOWN)) {

        // Get the occupied squares
        Bitboard occ = pos.occupied();

        // Check if there is a pawn on the right side of the epSquare that can move diagonally
        bool rightEp = (step<UP_RIGHT>(pawns & moveD)).get(epSquare);
        // Check if there is a pawn on the right side of the epSquare that can move anti-diagonally
        bool leftEp = (step<UP_LEFT>(pawns & moveA)).get(epSquare);

        // If there is a pawn on the right side
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

        // If there is a pawn on the left side
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

    // Calculate all safe squares that the king can move to
    Bitboard kingTarget = kingMasks[king] & safeSquares;

    if constexpr (!capturesOnly) {

        // Generate all legal king moves to squares that are empty
        Bitboard kingQuiets = kingTarget & empty;

        while (kingQuiets) {
            *moves++ = Move(king, kingQuiets.popLsb());
        }
    }

    // Generate all legal king moves to squares that contain enemy pieces
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
    // Define enemy color
    constexpr Color enemyColor = EnemyColor<color>();

    // Define friendly king square
    Square king = pos.pieces<color, KING>().lsb();
    assert(king != NULL_SQUARE);

    // Define bitboards used for move generation
    Bitboard friendlyPieces = pos.friendly<color>();
    Bitboard empty = pos.empty();
    Bitboard enemy = pos.enemy<color>();
    Bitboard occupied = pos.occupied();
    Bitboard checkers = getAttackers<color>(pos, king);

    occupied.clear(king);
    Bitboard safeSquares = ~getAttackedSquares<enemyColor>(pos, occupied);
    occupied.set(king);

    // Generate checkMask
    Bitboard checkMask = generateCheckMask(pos, king, checkers);

    // Generate king moves
    moves = generateKingMoves<capturesOnly>(pos, moves, king, safeSquares, empty, enemy);

    // If we are in a double check, only king moves are legal
    if (checkMask == 0)
        return moves;

    // Generate pinMasks
    Bitboard seenSquares = pieceAttacks<QUEEN>(king, occupied);
    Bitboard possiblePins = seenSquares & friendlyPieces;

    occupied ^= possiblePins;

    // Get all the pinners
    Bitboard possiblePinners = (pieceAttacks<QUEEN>(king, occupied) ^ seenSquares) & enemy;
    Bitboard pinners = ((pieceAttacks<ROOK>(king, occupied) & pos.pieces<ROOK>()) |
                        (pieceAttacks<BISHOP>(king, occupied) & pos.pieces<BISHOP>()) |
                        (pieceAttacks<QUEEN>(king, occupied) & pos.pieces<QUEEN>())) &
                       possiblePinners;

    // Define bitboards used for storing pin information
    Bitboard pinH, pinV, pinD, pinA, pinHV, pinDA, moveH, moveV, moveD, moveA;

    // Calculate pins
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

    // Generate pawn moves
    moves = generatePawnMoves<color, capturesOnly>(pos, moves, king, checkMask, moveH, moveV, moveD, moveA);

    // Generate knight and slider moves
    Bitboard sliderAndJumperPieces = friendlyPieces & ~pos.pieces<PAWN>();
    sliderAndJumperPieces.clear(king);

    moves = generateSliderAndJumpMoves<capturesOnly>(pos, moves, sliderAndJumperPieces, occupied, empty, enemy,
                                                     checkMask, pinHV, pinDA);

    // Generate castling moves
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