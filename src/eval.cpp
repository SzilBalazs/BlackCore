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

template<Color color>
Value evalPawns(const Position &pos) {
    constexpr Color enemyColor = EnemyColor<color>();

    Bitboard allPawn = pos.pieces<PAWN>();
    Bitboard ownPawns = pos.pieces<color, PAWN>();
    Bitboard enemyPawns = pos.pieces<enemyColor, PAWN>();

    Value value = PIECE_VALUES[PAWN] * ownPawns.popCount();

    if constexpr (color == WHITE) {
        Bitboard _wPawns = ownPawns;
        while (_wPawns) {
            Square square = _wPawns.popLsb();
            value += wPawnTable[square];
        }
    } else {
        Bitboard _bPawns = ownPawns;

        while (_bPawns) {
            Square square = _bPawns.popLsb();
            value += bPawnTable[square];
        }
    }
    return value;
}

template<Color color>
Value evalKnights(const Position &pos) {
    constexpr Color enemyColor = EnemyColor<color>();

    Bitboard knights = pos.pieces<color, KNIGHT>();

    Value value = PIECE_VALUES[KNIGHT] * knights.popCount();

    while (knights) {
        value += knightTable[knights.popLsb()];
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
    if constexpr (color == WHITE) {
        Square king = pos.pieces<WHITE, KING>().lsb();
        return wKingTable[king];
    } else {
        Square king = pos.pieces<BLACK, KING>().lsb();
        return bKingTable[king];
    }
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

    Score s = whiteEval.mg - blackEval.mg;


    return TEMPO_SCORE.mg + (pos.getSideToMove()==WHITE?s:-s);
}