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

#pragma once

#include "fathom/src/tbprobe.h"
#include "position.h"
#include "uci.h"

inline unsigned int TBProbe(const Position &pos) {
    // Initialize bitboards for friendly white and black pieces
    Bitboard white = pos.friendly<WHITE>();
    Bitboard black = pos.friendly<BLACK>();

    // Check if the total number of pieces on the board is greater than the largest loaded entry by the tablebase
    if ((white | black).popCount() > (int) TB_LARGEST)
        return TB_RESULT_FAILED;

    // Get the en passant square or 0 if it doesn't exist
    unsigned int ep = pos.getEpSquare() == NULL_SQUARE ? 0 : int(pos.getEpSquare());

    // Call Fathom's probe wdl function to get the WDL result for the position
    return tb_probe_wdl(white.bb, black.bb, pos.pieces<KING>().bb, pos.pieces<QUEEN>().bb, pos.pieces<ROOK>().bb,
                        pos.pieces<BISHOP>().bb, pos.pieces<KNIGHT>().bb, pos.pieces<PAWN>().bb,
                        pos.getMove50(), pos.getCastlingRights(), ep, pos.getSideToMove() == WHITE);
}

template<bool print>
inline bool TBProbeRoot(const Position &pos) {
    // Initialize bitboards for friendly white and black pieces
    Bitboard white = pos.friendly<WHITE>();
    Bitboard black = pos.friendly<BLACK>();

    // Check if the total number of pieces on the board is greater than the largest loaded entry by the tablebase
    if ((white | black).popCount() > (int) TB_LARGEST)
        return false;

    // Get the en passant square or 0 if it doesn't exist
    unsigned int ep = pos.getEpSquare() == NULL_SQUARE ? 0 : int(pos.getEpSquare());

    // Call Fathom's root probing function to get the misc information about the position including WDL, DTZ, best move
    unsigned int result = tb_probe_root(white.bb, black.bb, pos.pieces<KING>().bb, pos.pieces<QUEEN>().bb, pos.pieces<ROOK>().bb,
                                        pos.pieces<BISHOP>().bb, pos.pieces<KNIGHT>().bb, pos.pieces<PAWN>().bb,
                                        pos.getMove50(), pos.getCastlingRights(), ep, pos.getSideToMove() == WHITE, nullptr);

    // Check for failed, stalemate, or checkmate results
    if (result == TB_RESULT_FAILED || result == TB_RESULT_STALEMATE || result == TB_RESULT_CHECKMATE)
        return false;

    // Extract the from, to, and promotion squares from the result
    Square from = static_cast<Square>(TB_GET_FROM(result));
    Square to = static_cast<Square>(TB_GET_TO(result));
    unsigned int promo = TB_GET_PROMOTES(result);

    // Determine the promotion type
    unsigned char flags;
    switch (promo) {
        case 0:
            flags = 0;
            break;
        case 1:
            flags = PROMO_QUEEN;
            break;
        case 2:
            flags = PROMO_ROOK;
            break;
        case 3:
            flags = PROMO_BISHOP;
            break;
        case 4:
            flags = PROMO_KNIGHT;
            break;
        default:
            out("info string Unable to determine DTZ move promotion type!");
            return false;
    }

    // Get the WDL (Win, Draw, Loss) value
    unsigned int wdl = TB_GET_WDL(result);
    Score score;
    if (wdl == TB_WIN) {
        score = TB_WIN_SCORE;
    } else if (wdl == TB_LOSS) {
        score = TB_LOSS_SCORE;
    } else if (wdl == TB_DRAW || wdl == TB_CURSED_WIN || wdl == TB_BLESSED_LOSS) {
        score = DRAW_VALUE;
    } else {
        out("info string Unable to determine WDL!");
        return false;
    }

    if (print) {
        // Output information to the GUI
        out("info", "depth", 1, "seldepth", 1, "nodes", 1, "tbhits", 1, "score", "cp", score, "time", 1, "pv", Move(from, to, flags));
        out("bestmove", Move(from, to, flags));
    }
    return true;
}
