// BlackCore is a chess engine
// Copyright (c) 2023 SzilBalazs
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

#pragma once

#include "position.h"

/*
 * Static-Exchange-Evaluation
 *
 * Returns the material change, after playing out every resulting capture of the move.
 */
bool see(const Position &pos, Move move, Score threshold) {
    assert(move.isCapture()); // Make sure move is a capture

    Square from = move.getFrom();
    Square to = move.getTo();

    // TODO EP captures
    if (move.isPromo()) return true;

    Score value = PIECE_VALUES[pos.pieceAt(to).type] - threshold;

    if (value < 0) return false;

    value -= PIECE_VALUES[pos.pieceAt(from).type];

    if (value >= 0) return true;

    Bitboard rooks = pos.pieces<ROOK>() | pos.pieces<QUEEN>();
    Bitboard bishops = pos.pieces<BISHOP>() | pos.pieces<QUEEN>();
    Bitboard occ = pos.occupied() ^ Bitboard(from) ^ Bitboard(to);

    // Initialize the current attacker as the piece that made the capture
    Bitboard attacker = from;
    // Get all attackers to the destination square
    Bitboard attackers = pos.getAllAttackers(to, occ);

    Color stm = EnemyColor(pos.pieceAt(from).color);

    while (true) {
        attackers &= occ;

        PieceType type;
        attacker = pos.leastValuablePiece(attackers, stm, type);

        if (!attacker)
            break;

        value = -value - 1 - PIECE_VALUES[type];
        stm = EnemyColor(stm);

        if (value >= 0) {
            if (type == KING && (attackers & pos.friendly(stm))) {
                stm = EnemyColor(stm);
            }
            break;
        }


        occ ^= attacker;

        if (type == ROOK || type == QUEEN)
            attackers |= rookAttacks(to, occ) & rooks & occ;
        if (type == PAWN || type == BISHOP || type == QUEEN)
            attackers |= bishopAttacks(to, occ) & bishops & occ;
    }

    return stm != pos.pieceAt(from).color;
}