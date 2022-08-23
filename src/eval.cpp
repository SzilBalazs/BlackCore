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

#include "eval.h"

constexpr Bitboard WK_AREA = 0xe0e0e0ULL;
constexpr Bitboard WK_SHIELD_1 = 0xe000ULL;
constexpr Bitboard WK_SHIELD_2 = 0xe00000ULL;

constexpr Bitboard WQ_AREA = 0x70707ULL;
constexpr Bitboard WQ_SHIELD_1 = 0x700ULL;
constexpr Bitboard WQ_SHIELD_2 = 0x70000ULL;

constexpr Bitboard BK_AREA = 0xe0e0e00000000000ULL;
constexpr Bitboard BK_SHIELD_1 = 0xe0000000000000ULL;
constexpr Bitboard BK_SHIELD_2 = 0xe00000000000ULL;

constexpr Bitboard BQ_AREA = 0x707070000000000ULL;
constexpr Bitboard BQ_SHIELD_1 = 0x7000000000000ULL;
constexpr Bitboard BQ_SHIELD_2 = 0x70000000000ULL;

template<Color color>
Value evalPawns(const Position &pos) {
    constexpr Color enemyColor = EnemyColor<color>();

    Bitboard ownPawns = pos.pieces<color, PAWN>();
    Bitboard enemyPawns = pos.pieces<enemyColor, PAWN>();

    Value value = PIECE_VALUES[PAWN] * ownPawns.popCount();

    if constexpr (color == WHITE) {
        Bitboard _wPawns = ownPawns;
        while (_wPawns) {
            Square square = _wPawns.popLsb();

            // Double pawns
            if (fileMask(square) & ownPawns) {
                value += PAWN_DOUBLE_PENALTY;
            }

            // Passed pawn
            if (!(adjacentNorthMask(square) & enemyPawns)) {
                value += PAWN_PASSED_BONUS;
            }

            // Isolated pawn
            if (!(adjacentFileMask(square) & ownPawns)) {
                value += PAWN_ISOLATED_PENALTY;
            }

            value += wPawnTable[square];
        }
    } else {
        Bitboard _bPawns = ownPawns;

        while (_bPawns) {
            Square square = _bPawns.popLsb();

            // Double pawns
            if (fileMask(square) & ownPawns) {
                value += PAWN_DOUBLE_PENALTY;
            }

            // Passed pawn
            if (!(adjacentSouthMask(square) & enemyPawns)) {
                value += PAWN_PASSED_BONUS;
            }

            // Isolated pawn
            if (!(adjacentFileMask(square) & ownPawns)) {
                value += PAWN_ISOLATED_PENALTY;
            }

            value += bPawnTable[square];
        }
    }
    return value;
}

template<Color color>
Value evalKnights(const Position &pos) {
    constexpr Color enemyColor = EnemyColor<color>();
    constexpr Direction UP_LEFT = enemyColor == WHITE ? NORTH_WEST : -NORTH_WEST;
    constexpr Direction UP_RIGHT = enemyColor == WHITE ? NORTH_EAST : -NORTH_EAST;


    Bitboard enemyPawns = pos.pieces<enemyColor, PAWN>();
    Bitboard safe = ~(step<UP_LEFT>(enemyPawns) | step<UP_RIGHT>(enemyPawns));

    Bitboard knights = pos.pieces<color, KNIGHT>();

    Value value = PIECE_VALUES[KNIGHT] * knights.popCount();

    while (knights) {
        Square square = knights.popLsb();
        value += KNIGHT_MOBILITY * (knightMask(square) & safe).popCount();
    }

    return value;

}

template<Color color>
Value evalBishops(const Position &pos) {
    constexpr Color enemyColor = EnemyColor<color>();

    Bitboard pawns = pos.pieces<PAWN>();
    Bitboard bishops = pos.pieces<color, BISHOP>();

    Value value = PIECE_VALUES[BISHOP] * bishops.popCount();

    while (bishops) {
        Square square = bishops.popLsb();
        Bitboard attacks = bishopAttacks(square, pawns) & sideBB[enemyColor];
        value += BISHOP_ATTACK_BONUS * attacks.popCount();
    }

    return value;
}

template<Color color>
Value evalRooks(const Position& pos) {

    Bitboard pawns = pos.pieces<PAWN>();
    Bitboard rooks = pos.pieces<color, ROOK>();

    Value value = PIECE_VALUES[ROOK] * rooks.popCount();

    while (rooks) {
        Square square = rooks.popLsb();
        value += ROOK_MOBILITY * rookAttacks(square, pos.occupied()).popCount();

        int pawnsOnFile = (fileMask(square) & pawns).popCount();

        if (pawnsOnFile == 0) {
            value += ROOK_OPEN_BONUS;
        } else if (pawnsOnFile == 1) {
            value += ROOK_HALF_BONUS;
        }
    }

    return value;
}

template<Color color>
Value evalQueens(const Position& pos) {

    Bitboard queens = pos.pieces<color, QUEEN>();

    Value value = PIECE_VALUES[QUEEN] * queens.popCount();

    return value;
}

template<Color color>
Value evalKings(const Position& pos) {

    Bitboard pawns = pos.pieces<color, PAWN>();
    Bitboard rooks = pos.pieces<color, ROOK>();

    Value value;

    if constexpr (color == WHITE) {
        Square king = pos.pieces<WHITE, KING>().lsb();

        // We castled king side
        if (WK_AREA.get(king)) {

            // King shield
            value += KING_SHIELD_1 * (pawns & WK_SHIELD_1).popCount();
            value += KING_SHIELD_2 * (pawns & WK_SHIELD_2).popCount();

            // Trapped rook on the edge of the boards
            if (rooks.get(H1))
                value += ROOK_TRAPPED;

        }
        // We castled queen side
        else if (WQ_AREA.get(king)) {

            // King shield
            value += KING_SHIELD_1 * (pawns & WQ_SHIELD_1).popCount();
            value += KING_SHIELD_2 * (pawns & WQ_SHIELD_2).popCount();

            // Trapped rook on the edge of the boards
            if (rooks.get(A1))
                value += ROOK_TRAPPED;

        } else {
            value += KING_UNSAFE;
        }

        value.eg += egKingTable[king];

    } else {
        Square king = pos.pieces<BLACK, KING>().lsb();

        // We castled king side
        if (BK_AREA.get(king)) {

            // King shield
            value += KING_SHIELD_1 * (pawns & BK_SHIELD_1).popCount();
            value += KING_SHIELD_2 * (pawns & BK_SHIELD_2).popCount();

            // Trapped rook on the edge of the boards
            if (rooks.get(H8))
                value += ROOK_TRAPPED;

        }
        // We castled queen side
        else if (BQ_AREA.get(king)) {

            // King shield
            value += KING_SHIELD_1 * (pawns & BQ_SHIELD_1).popCount();
            value += KING_SHIELD_2 * (pawns & BQ_SHIELD_2).popCount();

            // Trapped rook on the edge of the boards
            if (rooks.get(A8))
                value += ROOK_TRAPPED;

        } else {
            value += KING_UNSAFE;
        }

        value.eg += egKingTable[king];
    }

    return value;
}

Score eval(const Position &pos) {

    Value whiteEval, blackEval;

    whiteEval += evalPawns<WHITE>(pos);
    whiteEval += evalKnights<WHITE>(pos);
    whiteEval += evalBishops<WHITE>(pos);
    whiteEval += evalRooks<WHITE>(pos);
    whiteEval += evalQueens<WHITE>(pos);
    whiteEval += evalKings<WHITE>(pos);

    blackEval += evalPawns<BLACK>(pos);
    blackEval += evalKnights<BLACK>(pos);
    blackEval += evalBishops<BLACK>(pos);
    blackEval += evalRooks<BLACK>(pos);
    blackEval += evalQueens<BLACK>(pos);
    blackEval += evalKings<BLACK>(pos);

    Value value = whiteEval - blackEval;

    int phase = 0;
    phase += pos.pieces<KNIGHT>().popCount();
    phase += pos.pieces<BISHOP>().popCount();
    phase += 2 * pos.pieces<ROOK>().popCount();
    phase += 4 * pos.pieces<QUEEN>().popCount();

    // Formula from https://www.chessprogramming.org/Tapered_Eval
    Score score = ((value.eg * (24 - phase)) + (value.mg * phase)) / 24;


    return TEMPO_SCORE + (pos.getSideToMove()==WHITE?score:-score);
}