#ifndef BLACKCORE_POSITION_H
#define BLACKCORE_POSITION_H


#include "bitboard.h"
#include "utils.h"

class Position {
public:
    Color stm;
    Square epSquare;
    unsigned char castlingRights{};

    constexpr Piece pieceAt(Square square) { return board[square]; }

    // This will be mostly used with constant color and type so this will result a nicer code
    // pieces<{ROOK, WHITE}>() --> pieces<WHITE, ROOK>()
    template<Color color, PieceType type>
    constexpr Bitboard Pieces() { return pieceBB[type] & allPieceBB[color]; }

    template<Color color>
    constexpr Bitboard Us() { return allPieceBB[color]; }

    template<Color color>
    constexpr Bitboard Enemy() { return allPieceBB[EnemyColor<color>()]; }

    template<Color color>
    constexpr Bitboard EnemyOrEmpty() { return ~Us<color>(); }

    void displayBoard();

    void loadPositionFromFen(const std::string &fen);

    Position();

    Position(const std::string &fen);

private:

    void clearSquare(Square square);

    void setSquare(Square square, Piece piece);

    void clearPosition();

    Piece board[64];

    Bitboard pieceBB[6], allPieceBB[2];

};


#endif //BLACKCORE_POSITION_H
