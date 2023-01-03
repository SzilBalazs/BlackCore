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

#ifndef BLACKCORE_EGTB_H
#define BLACKCORE_EGTB_H

#include "constants.h"
#include "fathom/src/tbprobe.h"

inline Score TBProbe(const Position &pos) {
    Bitboard white = pos.friendly<WHITE>();
    Bitboard black = pos.friendly<BLACK>();

    if ((white | black).popCount() > TB_LARGEST)
        return UNKNOWN_SCORE;

    unsigned int ep = pos.getEpSquare() == NULL_SQUARE ? 0 : int(pos.getEpSquare());
    unsigned int result = tb_probe_wdl(white.bb, black.bb, pos.pieces<KING>().bb, pos.pieces<QUEEN>().bb, pos.pieces<ROOK>().bb,
                                       pos.pieces<BISHOP>().bb, pos.pieces<KNIGHT>().bb, pos.pieces<PAWN>().bb,
                                       pos.getMove50(), pos.getCastlingRights(), ep, pos.getSideToMove() == WHITE);

    if (result == TB_WIN) {
        return TB_WIN_SCORE;
    } else if (result == TB_LOSS) {
        return TB_LOSS_SCORE;
    } else if (result == TB_DRAW || result == TB_CURSED_WIN || result == TB_BLESSED_LOSS) {
        return DRAW_VALUE;
    } else {
        return UNKNOWN_SCORE;
    }
}

inline bool TBProbeRoot(const Position &pos) {
    Bitboard white = pos.friendly<WHITE>();
    Bitboard black = pos.friendly<BLACK>();

    if ((white | black).popCount() > TB_LARGEST)
        return {};

    unsigned int ep = pos.getEpSquare() == NULL_SQUARE ? 0 : int(pos.getEpSquare());
    unsigned int result = tb_probe_root(white.bb, black.bb, pos.pieces<KING>().bb, pos.pieces<QUEEN>().bb, pos.pieces<ROOK>().bb,
                                        pos.pieces<BISHOP>().bb, pos.pieces<KNIGHT>().bb, pos.pieces<PAWN>().bb,
                                        pos.getMove50(), pos.getCastlingRights(), ep, pos.getSideToMove() == WHITE, nullptr);

    if (result == TB_RESULT_FAILED || result == TB_RESULT_STALEMATE || result == TB_RESULT_CHECKMATE)
        return {};

    Square from = static_cast<Square>(TB_GET_FROM(result));
    Square to = static_cast<Square>(TB_GET_TO(result));
    unsigned int promo = TB_GET_PROMOTES(result);

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
            return {};
    }
    
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
    
    out("info", "depth", 1, "seldepth", 0, "nodes", 0, "score", score, "time",
        1, "pv", Move(from, to, flags));
    out("bestmove", Move(from, to, flags));
    return true;
}

#endif //BLACKCORE_EGTB_H
