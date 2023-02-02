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

#ifndef BLACKCORE_CONSTANTS_H
#define BLACKCORE_CONSTANTS_H

#include <string>

// clang-format off

// #define TUNE

// #define DATA_FILTER

#if defined(NATIVE) && defined(__BMI2__)
#define BMI2
#endif

#if defined(NATIVE) && defined(__AVX2__)
#define AVX2
#endif

typedef uint64_t U64;
typedef int32_t Score;
typedef int8_t Depth;
typedef int8_t Ply;

constexpr Score UNKNOWN_SCORE = 100002;
constexpr Score INF_SCORE = 100001;
constexpr Score MATE_VALUE = 100000;
constexpr Score TB_WIN_SCORE = 50000;
constexpr Score TB_WORST_WIN = 49000;
constexpr Score TB_BEST_LOSS = -49000;
constexpr Score TB_LOSS_SCORE = -50000;
constexpr Score WORST_MATE = MATE_VALUE - 100;
constexpr Score DRAW_VALUE = 0;

const Ply MAX_PLY = 100;

const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

constexpr unsigned int RANDOM_SEED = 1254383;

enum Square : int {
    A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7,
    A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15,
    A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23,
    A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31,
    A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39,
    A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47,
    A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55,
    A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63,
    NULL_SQUARE = 64
};

constexpr Square flipSquare(Square sq) {
    return Square(int(sq) ^ 56);
}

constexpr unsigned char WK_MASK = 1;
constexpr unsigned char WQ_MASK = 2;
constexpr unsigned char BK_MASK = 4;
constexpr unsigned char BQ_MASK = 8;

inline Square operator+(Square &a, int b) {
    return Square(int(a) + b);
}

inline Square operator-(Square &a, int b) {
    return Square(int(a) - b);
}

inline Square operator+=(Square &a, int b) {
    return a = a + b;
}

inline Square operator-=(Square &a, int b) {
    return a = a - b;
}

// Function that converts an uci format square string into an actual square.
inline Square stringToSquare(std::string s) {
    if (s[0] == '-') {
        return NULL_SQUARE;
    } else if ('a' <= s[0] && s[0] <= 'z') {
        return Square((s[0] - 'a') + (s[1] - '1') * 8);
    } else if ('A' <= s[0] && s[0] <= 'Z') {
        return Square((s[0] - 'A') + (s[1] - '1') * 8);
    }

    return NULL_SQUARE;
}

inline std::string asciiColor(int a) {
    return "\u001b[38;5;" + std::to_string(a) + "m";
}

constexpr std::string ASCII_RESET = "\u001b[0m";
constexpr std::string ASCII_WHITE_PIECE = "\u001b[90;107m";
constexpr std::string ASCII_BLACK_PIECE = "\u001b[100;97m";

inline std::istream &operator>>(std::istream &is, Square &square) {
    std::string s;
    is >> s;

    square = stringToSquare(s);

    return is;
}

enum LineType : int {
    HORIZONTAL = 0,
    VERTICAL = 1,
    DIAGONAL = 2,
    ANTI_DIAGONAL = 3
};

enum Direction : int {
    NORTH = 8,
    WEST = -1,
    SOUTH = -8,
    EAST = 1,
    NORTH_EAST = 9,
    NORTH_WEST = 7,
    SOUTH_WEST = -9,
    SOUTH_EAST = -7
};

constexpr Direction DIRECTIONS[8] = {NORTH, WEST, NORTH_EAST, NORTH_WEST, SOUTH, EAST, SOUTH_WEST, SOUTH_EAST};

constexpr Direction opposite(Direction direction) {
    return Direction(-direction);
}

constexpr Direction operator-(Direction direction) {
    return opposite(direction);
}

enum PieceType {
    PIECE_EMPTY = 6,
    KING = 0,
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5
};

enum Color {
    COLOR_EMPTY = 2,
    WHITE = 0,
    BLACK = 1
};

enum NodeType {
    ROOT_NODE,
    PV_NODE,
    NON_PV_NODE
};

constexpr PieceType PIECE_TYPES_BY_VALUE[6] = {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};

struct Piece {
    PieceType type = PIECE_EMPTY;
    Color color = COLOR_EMPTY;

    constexpr Piece() = default;

    constexpr Piece(PieceType t, Color c) {
        type = t;
        color = c;
    }

    constexpr bool isNull() const {
        return type == PIECE_EMPTY || color == COLOR_EMPTY;
    }
};

// 2 colors * 6 types * 64 square = 768
// 4 for castling rights
// 8 number for the file of the epSquare
// 1 number if the side is black

const U64 randTable[781] = {
        0x4ef488bfae17abbaULL, 0x1b608e38d5cf7308ULL,
        0x851dabc8d9c029daULL, 0x62ff3fd5ca1fe189ULL,
        0xb58f435e51ec54d9ULL, 0xd6478aa5957c39eaULL,
        0x1a84642313330ed8ULL, 0x24b6ff164e7af0f7ULL,
        0x6be96adf24b3816cULL, 0xece099a9b0d53073ULL,
        0xdb8346d2cf6a823ULL, 0x7010f7da6962efd5ULL,
        0xc70fcf5e7ac61c40ULL, 0x69c81f400aacb213ULL,
        0x543deca39cd4669cULL, 0xbccbf8f1ac568f1aULL,
        0x1b657f6eb9085b4dULL, 0x8dbc14bcc7085c8eULL,
        0x31b11a994768a079ULL, 0xdb65d6222d174223ULL,
        0x69a37f411c1015ULL, 0x7277d9df84e80ce3ULL,
        0xffb2d94597293905ULL, 0xd6c7002205dad90aULL,
        0xec0979ba90e44095ULL, 0x8f7f5e9303057addULL,
        0x4ba8f1c0656c62a6ULL, 0x1b40d16f768690dcULL,
        0x51166315ea155c2cULL, 0x33aad4f2bc026d3fULL,
        0x54dafe930aa273d0ULL, 0x1d885cc020e4e77aULL,
        0xf8eb51befffbede4ULL, 0xefa21aa88e2ad467ULL,
        0x1e639296082a76b6ULL, 0x95d9c8ebce333424ULL,
        0x5ea0892946799861ULL, 0xc0930a65ce836ab4ULL,
        0x5a674325485b4e7fULL, 0x8b99f12278470c72ULL,
        0xa4e008d3283c94e8ULL, 0x6fbe24c1740a6981ULL,
        0xea1273d134b0f010ULL, 0x152d52d9c30c78efULL,
        0x9dc76a5d5c289ae0ULL, 0xc5c6dbe7f09411a7ULL,
        0x2073418d01c03687ULL, 0xa0c17a6f4f985f7ULL,
        0x7d9c1720beb9db73ULL, 0x6849424ca595606dULL,
        0xb29fba8417b80df4ULL, 0x61f12e84c28750aeULL,
        0x8bf8fcd9f3f1b1edULL, 0xd0ce225a433287c4ULL,
        0x775917e60886c9e3ULL, 0x486ada4af87cc0abULL,
        0xee6da9bca06fa88eULL, 0x24887e607e1c950dULL,
        0xbad992958c65ef8eULL, 0x66563aeb0c4728c8ULL,
        0x140f0bfc12d599c5ULL, 0x1a8a7286be1ad376ULL,
        0x965c145f47a5e3d8ULL, 0x3bfd4d07824890c4ULL,
        0x8e9e7a0370aa060eULL, 0xc8d523c52edd3d75ULL,
        0x3cfca5a158ac9985ULL, 0x9adc9ad634e45bcbULL,
        0x310f96adc5796c5cULL, 0x96b7c3b005c1d41bULL,
        0xefc6b36f68cd44b9ULL, 0xb9e32448740802e0ULL,
        0xe41875586b23a7ccULL, 0xdf11200a2af368eeULL,
        0xccce23fb87e01cb9ULL, 0x6cd74548000380deULL,
        0x2068ab7c48d2ec38ULL, 0x3f8d34605d6d160dULL,
        0x2f81480ab31791beULL, 0x70dbffe0f42c55a9ULL,
        0x464b1ccca9b60873ULL, 0xd8197bb6610f4a26ULL,
        0xc9285e20789da7cULL, 0xd5735a684fc9656ULL,
        0x5b2c4e2fb331fd65ULL, 0x7c307e89787b59a1ULL,
        0xfd67c65dcf80bc5fULL, 0x89aca3d35d4f61e0ULL,
        0x67b203f3de779d44ULL, 0xa39d8dd5b5cbbc6ULL,
        0x33524df0e5afbd58ULL, 0x537f4763dc8ff6dULL,
        0x9a5798c41a86ae87ULL, 0x3794e55b391f0b97ULL,
        0xb154b14aac830d0ULL, 0x5caa36bbe5a0e263ULL,
        0x900f52b8661fcb0eULL, 0x957561cdd87be82fULL,
        0xdfbe8f1da18c3111ULL, 0x7a6f1024756f626bULL,
        0x5c4ab867aee28e4fULL, 0xbdaf1cd791244211ULL,
        0x2228f2f8d4f28439ULL, 0x4edd616d5aad4c8ULL,
        0xb27b2f3225769f69ULL, 0x1425c27823849280ULL,
        0xae76ce5d77c504eeULL, 0x2779747ca414ebc8ULL,
        0xe4ef214a300c5c0dULL, 0xed4c07343bd0d849ULL,
        0x4f9b26a8a371df7cULL, 0x303562d4fd5501bdULL,
        0x3f0e099b1dd1ab96ULL, 0x8500302b98f9de08ULL,
        0x7b9dad80e5c7b10ULL, 0x9ccdf0c5e8245906ULL,
        0xfee6417294e52bd3ULL, 0x6a85cc0e7dfd0defULL,
        0x4aab0010efaff4d7ULL, 0x8b0ac3450ebc6a6eULL,
        0xe5aaa01795063e90ULL, 0x87378e42e097549bULL,
        0x2f7942b8aada0a9dULL, 0x834bad7e2a7087a3ULL,
        0x2102570d02344402ULL, 0x15ccf7864011db09ULL,
        0x69a3f87974b90867ULL, 0xbc061d452ff48a89ULL,
        0xa8b23bf0a0ff3926ULL, 0x4eff3a36103d3ba0ULL,
        0x5d85348b3bcae7b2ULL, 0xb93e7a1145e5b5dcULL,
        0xa5d7fbf64201e5e4ULL, 0x16a0dc5777406be9ULL,
        0x11705137b3f7dbb1ULL, 0xf9c5c349d5216f0eULL,
        0x7ff95761cf184259ULL, 0x51a4d1ccb7afdeceULL,
        0x73e35224634a880ULL, 0x6486ba6de09e55b7ULL,
        0xec2e1735a409693bULL, 0x4a32aa8e00d54148ULL,
        0xd1ad6d6f57b14c0cULL, 0xbb8c972637c64696ULL,
        0xc1367bf134c6785cULL, 0x4f5fa35416f09e99ULL,
        0x8ff1a2f1c6a485acULL, 0x309e4941e7a51b39ULL,
        0xbb015739220065f7ULL, 0x15c93d157f460ca8ULL,
        0x9b5e446785a25be9ULL, 0xdf672b8e44dccffaULL,
        0xc6f0e8348fdde7a5ULL, 0xdb72d3ffa3740dabULL,
        0x27a907bf1d1aba62ULL, 0x4bfe65e196423003ULL,
        0xa52e8487e57ec77aULL, 0x2cb75b88afd3dbdfULL,
        0x15f1fe898c111f19ULL, 0x1d572ea679042ce0ULL,
        0xe130479b204d25dbULL, 0x412ce0d560b048f8ULL,
        0x941173a2d0c645f1ULL, 0xd4b541013f264760ULL,
        0x3427e4d5d1a2a27ULL, 0x4bcaa0619e8e733bULL,
        0xa13b6e82e52f1643ULL, 0x722ba71b722670cdULL,
        0x38eb0d3ec3e77f62ULL, 0x1e7bb16186c390fdULL,
        0x8b1fc3896dfcef99ULL, 0xa5e23c9ebed4e8fcULL,
        0x7cd15354990c856dULL, 0x808d16c17a97b916ULL,
        0x3653a3d7864d8bbaULL, 0x54b0b596cdeb0f82ULL,
        0xcf3f60ffd38fb97bULL, 0x72e37da591c9b91fULL,
        0x84e18873e75898e4ULL, 0xeff11a5af6809bb0ULL,
        0x8af4ebace6b714ecULL, 0xaa4b1e42d5d10f26ULL,
        0xbcceef4a1feb7b40ULL, 0xf9ebb16e54eaf81bULL,
        0xb9aef6775240c929ULL, 0xb540bc9e71416e61ULL,
        0x518fe188eded4203ULL, 0x221d99c8f8bfd5b7ULL,
        0xc38ba67f3a338066ULL, 0xabc5069a1d6cc43ULL,
        0x21b22c043f2a8875ULL, 0xd74c9c5103fbfb3fULL,
        0xffcc2122caa78d7bULL, 0x76451830ceddee8ULL,
        0x247a6f2c0dd1586cULL, 0x52eef33635cacc7aULL,
        0xabe97ca6f9662cabULL, 0xa276321ccf1259ceULL,
        0xcb839d28b380f78ULL, 0x8055e3152e35e57ULL,
        0xbf59fb51057f5ac8ULL, 0x1b7d309c100f5b9aULL,
        0xb70ab3b1362f7ec3ULL, 0x87348440973488e5ULL,
        0x6af83f6e1d775c42ULL, 0x6cb5ba286d550eeaULL,
        0xfeb0c55e4eb7c047ULL, 0x9e20a0a79e40278fULL,
        0x4c4c42865d7c8c56ULL, 0xa8c5aecd505a8285ULL,
        0xa416d49733947e17ULL, 0x703a72e0113b1b15ULL,
        0x39770e959139b6a8ULL, 0x323ef9a4a3b209d4ULL,
        0xa8789a6b73541e98ULL, 0xe48d2b3a77fc3f6dULL,
        0x9a53f3778b46625eULL, 0x451b14e0dbe2ba7eULL,
        0xff936fc73fcb562cULL, 0x563f1a92ac3dab96ULL,
        0x8009fd8127ce5dd4ULL, 0x1fe44b04b27c4d74ULL,
        0x1bddbcb6dfd04c66ULL, 0xd1d59cc480da1ee6ULL,
        0xe0b2a33234409fa0ULL, 0x65c413574467da90ULL,
        0x979ccc89e5cf4b63ULL, 0x98bd96a4a29b3f31ULL,
        0x9917a96a7861c6a8ULL, 0x86fb59d30d1749f7ULL,
        0xd24e0e1e46e13ffcULL, 0xef47c0e7cb34eacdULL,
        0x3c3d8ad364a19d9fULL, 0xe1adf5f2eb3c58b8ULL,
        0xf090b59acd76efe5ULL, 0x90fec91f5ff633d4ULL,
        0xe8bc857e3f52e36bULL, 0x4339fadb39b0c022ULL,
        0x48ca28afd56c6022ULL, 0xb883f7d594e5feb1ULL,
        0x60647e12153912baULL, 0xcfba643175b17b1eULL,
        0xf49e02ccf10b3b18ULL, 0xa87c62b52eb20c74ULL,
        0xa3fe6658bf250c0cULL, 0x7f96fb396d7ae7aULL,
        0x8a04813c94856826ULL, 0xb4658d520f23975eULL,
        0xde333be245da49c0ULL, 0x31dff5ce643f1702ULL,
        0x949af87a549dab18ULL, 0x63335e199039c685ULL,
        0xd3bfb1f85b631a2cULL, 0x83fe4b755fd13d82ULL,
        0x2b9a11c97d74d5c2ULL, 0x96c1075ca256561aULL,
        0x9bd796371a7471cULL, 0x255c487ca55fd070ULL,
        0xb9716816c0e0d528ULL, 0x248723c56eeb7720ULL,
        0x610c2245f1921cULL, 0x3dcd62eb1d233973ULL,
        0xf6312646440e81c4ULL, 0x11fd03c380eab5baULL,
        0x5e55d9b4c19e5653ULL, 0x5e1f9c64cbe206b8ULL,
        0xc95323b729adc6e3ULL, 0xa5c0221d0f783a2bULL,
        0xb04d620a4d82cfebULL, 0xaf19ed57ccb129d0ULL,
        0xb8822c82928dd3e0ULL, 0xff6fd0c799541911ULL,
        0x752aafc4d762a830ULL, 0xf1118eb988c18b1cULL,
        0x1a6329e05224a736ULL, 0x9245204314dcdfe7ULL,
        0xed767f78545355d2ULL, 0x815acab3feb7900eULL,
        0x47f22a0cd634bc40ULL, 0xec2150afe4587f5bULL,
        0x3c484edd3f6f92d8ULL, 0x34a09aeada42e7eULL,
        0x29a434cf85fbdb04ULL, 0xd6c13eacdba32fecULL,
        0xa32d234290c51672ULL, 0xd301f3ada78249feULL,
        0xcd811f1efc2eead5ULL, 0x835457b34bab9e68ULL,
        0x181d24352631d82cULL, 0x8b45b912ca31d58dULL,
        0x1d6848348ec4ace5ULL, 0xc0aaf998beb88726ULL,
        0x1e8b4967c5af7c04ULL, 0xce8936ee45eefed9ULL,
        0x783c919b02ed3b82ULL, 0x72ce2fe88e153173ULL,
        0xd378f6a2df2c2e0cULL, 0xfb08aac2ee4a9e63ULL,
        0x1ec016a3ac2dbe0fULL, 0x475781f94fcbb7e3ULL,
        0x80f62a1e4ba4c50ULL, 0x10cf614e7adfc50bULL,
        0x32f64cbb84baa644ULL, 0x4db0d9f741b5cc38ULL,
        0x504344a0d07fda24ULL, 0x31c6d96248e05180ULL,
        0x8aab53db86275cacULL, 0x7e54f242474170f2ULL,
        0xa0d66308bd7bd65eULL, 0x3bd7fac1b8e5862ULL,
        0x516a4699f8ef4795ULL, 0xf1c9819a81809c9bULL,
        0xbb179cb7b69be105ULL, 0xa2997c2f69adb7acULL,
        0x3e2b82eee227f520ULL, 0x7f4814295541eb88ULL,
        0x3f95131c7938ae98ULL, 0x8c794a88fdaa5c96ULL,
        0xe995bd6613be075eULL, 0x168ecc747d13e52cULL,
        0xf34ee3443dee4bf9ULL, 0x42d5973ad0c37a75ULL,
        0x7b7eaecd8b56fddULL, 0x2fe24f61a33fc790ULL,
        0x64f78049d9027152ULL, 0xe67199262c305f69ULL,
        0x713b1b64d2383435ULL, 0xfee8d25b603ce2ddULL,
        0xd9adba56036e1351ULL, 0x5f2e7b36093e3f88ULL,
        0x6f9e79d8c47598ffULL, 0xb2312be3bf2eea4aULL,
        0xf2f2332baee0350fULL, 0xc1ece6dc79ab182dULL,
        0x8fd06333941a9fd6ULL, 0x629474ad192cacd5ULL,
        0x437a75202034b018ULL, 0xf9b98f21eea6f0caULL,
        0x2bf74641859777cfULL, 0xa02e3f2a08f19ef3ULL,
        0x1428f18c2a0bc0c8ULL, 0x1eb7a4e58ad098e0ULL,
        0xaed59e19b84f986aULL, 0x8e3f5245dbb77ef4ULL,
        0x9375c5327eb7d476ULL, 0x32782fef06c0cac7ULL,
        0xfc1a5392d7a43a6dULL, 0x5a5b8cf9810dbd00ULL,
        0xa8d0616cc96fcb70ULL, 0xeba216f0c7c22ea3ULL,
        0xf35625f70bf7d031ULL, 0x9d56c654583ce93bULL,
        0x1ae33a218238ea27ULL, 0xb865584a0811f9d9ULL,
        0xf9065d696d85514cULL, 0xee33a2c5133d068eULL,
        0xb4b2ab633b2ecd38ULL, 0x9cdc9a04cdc594ffULL,
        0x2f745ae57a3f043ULL, 0x1ede01f5037a92d2ULL,
        0xcef5984903f8984fULL, 0x183a79488d33e3b2ULL,
        0x1f3b9bf5c525fcc9ULL, 0x4169c9bdc608ff1cULL,
        0x8fdf3ca348d00c21ULL, 0x2b472034fde89860ULL,
        0x94ac5ca7c6e93e80ULL, 0x70fe25941302b0d1ULL,
        0xa6269b60f21ba380ULL, 0xcb518e0ae9c0aae4ULL,
        0x6e80ae4af54b28afULL, 0x79ceb17e48189126ULL,
        0xad89b6f68254de83ULL, 0xdda2c183a04d3489ULL,
        0xf6a67b91699bbb14ULL, 0xb445c2b018a1700ULL,
        0x4a50f28f965c2c5fULL, 0x1153b066f4d3889aULL,
        0xc86ec6e3ac241950ULL, 0xfe910b08c2bb060dULL,
        0x867496fded3c83e3ULL, 0xaebac214a1d7a1b3ULL,
        0xd65fb3b895a6d20ULL, 0x58aafe1d13a3ac78ULL,
        0xf47ce780dc44cf25ULL, 0x3e56c3611756c1feULL,
        0x3fb3e4f6d0483849ULL, 0x1a5c6b2e10ce205ULL,
        0xa9c529fd702edd6eULL, 0x869649faa5a1aee9ULL,
        0x1b8d911c776ec17cULL, 0x4e43b5428c8ed2abULL,
        0x775b758b4d9b3c9aULL, 0x8804d262fc071badULL,
        0x61d4695f713793b5ULL, 0x2db7ec74f5c05479ULL,
        0x664a809299686590ULL, 0xa8d033c5115e487eULL,
        0xe619ef70eb5a16acULL, 0xcd8b964a4281bfd5ULL,
        0xcef1aeef7d350c7fULL, 0x4674942f573a9ce5ULL,
        0x6dc0b11b30519e13ULL, 0x93df9452b5dcd8cbULL,
        0x9448f6f930deea98ULL, 0xed6eebca0706f29fULL,
        0x902200e6d2a3eb8bULL, 0x8729df3b219138acULL,
        0xcfc2bc0b6c88e014ULL, 0x5238e41b26599129ULL,
        0xee293f799e223f91ULL, 0xb1a2ef20a726114fULL,
        0x5f691e62fde20700ULL, 0x40b288b5aa28284cULL,
        0x37e0deebffaee7faULL, 0xba19fa6bbe3807e3ULL,
        0x4c6b7bfb9e9686a8ULL, 0xc7c761ba0e630b0dULL,
        0x12dd354400a464dbULL, 0x3e6cd81067e4545bULL,
        0x62f6f8ab23cc7855ULL, 0x60ef3c823b23d03aULL,
        0x3b8f1621794f84baULL, 0x5f317bcf29799a35ULL,
        0xd486d926a75263bcULL, 0xfa08e546eeabf2b5ULL,
        0x2de31bb003cc3076ULL, 0xcfd849eabb6bf6f8ULL,
        0x94d2f96c14d03c4eULL, 0x7cb7390dc796f98dULL,
        0x978a9733a3c105b3ULL, 0x6c7026ef2197c422ULL,
        0xbeb427a113122ee8ULL, 0x8522cbae77bc3afbULL,
        0xa013ef165581a060ULL, 0xddd17ae26c8f83fdULL,
        0x20ec4adcd53f5e2cULL, 0xa2a0d6c8e11de5bULL,
        0xd540e3516ac796b3ULL, 0x2cb963a350f1853dULL,
        0x3faae45402362ce7ULL, 0x32bbfccb615eaab1ULL,
        0x7d0833ac51ad1830ULL, 0x99305a7a7134e457ULL,
        0x5b989947a49efe86ULL, 0x6a6782e86aadc279ULL,
        0x66c06c92963784a3ULL, 0x95d5ee699932c142ULL,
        0x7c85986f37b8c1fbULL, 0x9c4fe3c2608495d2ULL,
        0xfd949ca2a7fd6654ULL, 0x8cc451cc93362187ULL,
        0x4502ee450cd8f8deULL, 0x350ca2eba9c817f0ULL,
        0xe5ed9484ecd2865ULL, 0xd6d294e1a63e1bd4ULL,
        0x407fe7fc78bf4a42ULL, 0x5d1e54565e373e99ULL,
        0x8f4c8f1539335ed0ULL, 0x9435975634b24565ULL,
        0xc4c11eb58a8b5ac6ULL, 0x8cf964566edaa61dULL,
        0x603299d26fa4409fULL, 0x4eaec27e571f9478ULL,
        0x2da410afdc0c7c07ULL, 0xa7b1c3237a1f7653ULL,
        0xd790d29141a56a3aULL, 0x64824a6cfa39ff28ULL,
        0x51883faf43fd25b2ULL, 0x5a938601203e32cbULL,
        0x3809ed67f923807fULL, 0xa6bdfc79447d1306ULL,
        0x79a001afbc97a49bULL, 0x501f11ac9d772e2cULL,
        0xdb7a2355f1f1c913ULL, 0xf2e018fa0a676207ULL,
        0xd451eea58e08137dULL, 0xf13a570d5ef737d6ULL,
        0x3f4ed84b0708f566ULL, 0xdc06604f709bc7bdULL,
        0xc4d17abee8b007aULL, 0x89b6a700afad7f15ULL,
        0x72f4003abba3549fULL, 0x7c0f7ec5cf3865fbULL,
        0xde391dec54a0d797ULL, 0xb45067f02bfe4b3cULL,
        0xea09e08c94bb3acULL, 0x329493d1e58875ecULL,
        0x1f6188a0d0c9765cULL, 0xf9780f58c8656f56ULL,
        0x73c62a39f796f99aULL, 0xeb625a8da0599a07ULL,
        0xf6aaef39e7e2b217ULL, 0x5fc0de127ee8b4c7ULL,
        0xf57798668b86ac45ULL, 0x454dcf315d5fa2d6ULL,
        0x80af178025adf8d0ULL, 0xd65da73ed5ef923ULL,
        0xb7a2ebe31805fa2cULL, 0xd12f9282362b2304ULL,
        0xcdf6cdd669f87400ULL, 0xa250eabbcfc5d25fULL,
        0xae00b05fb2ae4014ULL, 0xebc9edc4206fa6cbULL,
        0x7689ed2cadd75ec1ULL, 0x4a7e124305d9e14cULL,
        0x2c3bdc6dd2f17394ULL, 0x55eaf991d6f0feabULL,
        0x26fc46b584950b50ULL, 0xd1fd62345d4ce0e8ULL,
        0x8a23992ea3d7e47bULL, 0x875e0896d407b09fULL,
        0xc643961123bf68d8ULL, 0x1a94d8f1b3b6396ULL,
        0x8c0298fb53f4f45eULL, 0xe2852828f7f9fa2dULL,
        0x21dcadd35862a95cULL, 0x1bb936f6bf48665aULL,
        0x7adc2a8b206b4cfeULL, 0xc505891fc46c54bcULL,
        0x1409e52ff1155b44ULL, 0x21205a745fc6f119ULL,
        0x3ace40e876181924ULL, 0x358d1c0927d5e6a6ULL,
        0xd9367a5805c967caULL, 0x30b9ea14a65d5166ULL,
        0xad641697fcb57c65ULL, 0x1e1935296140ce21ULL,
        0xf6cb4244858f5c0ULL, 0x5bc33c52c56e7adaULL,
        0x7c240347ad29ee9dULL, 0x38c3d7c57cc6ebf9ULL,
        0x3d11adba2060fbdaULL, 0x7559ead850090e06ULL,
        0xfd5e13ca4082b63dULL, 0x166a5bb6945d5ef7ULL,
        0x5546f01c2b69bd55ULL, 0x5fd08bb5eff91708ULL,
        0x58c1f08abbd41db4ULL, 0x9472e8384245280ULL,
        0x773b5b811184e9daULL, 0x36009938bdf09510ULL,
        0x2f423987abbd7897ULL, 0xc7a428ad2b40569bULL,
        0x2db547e5c4eac72fULL, 0xbcd4710dc0294b65ULL,
        0xa1ecbd582f58ab6dULL, 0xf45a55c2bf631c3fULL,
        0x79c87516d4f89dd6ULL, 0xe63e61780d389467ULL,
        0x53ae6a68eb68bf1bULL, 0x231d521de4ea3eabULL,
        0x98dcf370807e85edULL, 0xcbd18662ec57627bULL,
        0x17ccb5409b5be7c5ULL, 0x916858886ad455e8ULL,
        0x12d5cfd28093fa1bULL, 0x849433c93b78ae1bULL,
        0x483de1bbcd54588bULL, 0xc4542529e1400253ULL,
        0xe7ba10aaac73d3d4ULL, 0x60716cc47639c8c0ULL,
        0x8d0f798f8016db29ULL, 0xc7f540363e5d38c0ULL,
        0x71f8eb2cbde3f2a4ULL, 0x73f51915cfb11591ULL,
        0x8e368be8220fbed8ULL, 0xe6634d8d5481c274ULL,
        0xfded89783c7d7d45ULL, 0x3a14056f6fb7aad6ULL,
        0x9c004929888c348ULL, 0xb47cb32c65003d99ULL,
        0x544263587ab108beULL, 0xe86fe92c1f254ad8ULL,
        0x5a757f4aa565ba2fULL, 0x6821483d80e763ffULL,
        0xbceaf85474a4a459ULL, 0x1cd6e6d20c667900ULL,
        0xe7b38fccff113c7bULL, 0x20361c9a7ec6b064ULL,
        0x99ff2481edd18564ULL, 0xb616e25d79d6642cULL,
        0xdf0ce9291722e828ULL, 0x491db70eb9314fb4ULL,
        0xab2bff7e330d3061ULL, 0x4232ed0a2e93d8a8ULL,
        0x6d55fcf2098eca3eULL, 0x909d9001b58e3f5eULL,
        0x82065263e6c495b2ULL, 0x54c2239d9e98cd65ULL,
        0xb5253fdbb715a77eULL, 0x7d4abc24bc60a674ULL,
        0xab5b6ed65d25fa03ULL, 0x56c7311e57cbd8e6ULL,
        0x865e2030ff154748ULL, 0x7f4e6b4a04ac3248ULL,
        0xf42dd3d5244b697aULL, 0xd7ecc66e36dcfb01ULL,
        0x6f5a6edeaf36f57fULL, 0xfa2d8db39c449a39ULL,
        0xe71efc1684cf44beULL, 0x75d9c8cff2c9e1c2ULL,
        0xe36a2caf0087bf36ULL, 0x79d5716971860e60ULL,
        0xf73afb6d93e9fc40ULL, 0xe60ff6946b7cc27bULL,
        0xd7a71a4be5007a5aULL, 0x47d93d3126c8823ULL,
        0x8d988ee64bcc0e9fULL, 0x7c3b9f53e45af01ULL,
        0x4588669519b5b9bULL, 0x1cb3098aa4122760ULL,
        0xd7903ac6afc8390ULL, 0x44bd7b928dd8e7e5ULL,
        0x578ac5e77f09c3a3ULL, 0x847e04f37d6998b1ULL,
        0x6ec83dfbd780d189ULL, 0x490810c885fd662bULL,
        0xe25841c565edee7fULL, 0x144ba2ed46b28fe2ULL,
        0x1066ddb06e90ee12ULL, 0xf5c5d6f35fe8e694ULL,
        0x73f84c285181738fULL, 0x8c7881debcd4ad91ULL,
        0x33a961f1cd1b1c0aULL, 0xbc75c5755cfecb81ULL,
        0xb2ac5cadbb9d8fd3ULL, 0x5621d4ab21a9a722ULL,
        0x51ba2205bb14e851ULL, 0x851c873f570b9de6ULL,
        0x78eecadacb3d0202ULL, 0x8933104e7337b965ULL,
        0x1f28bc249a2e5b46ULL, 0x7fd5c88533cbe11aULL,
        0xde58e22c6fee97caULL, 0xe3da2f25b2385cfeULL,
        0xec06b46697321104ULL, 0x1ae0829b639db3d5ULL,
        0x4bba7d7b0ad568b6ULL, 0xfaa78fbef3360dbbULL,
        0x5406355e1e3eeee1ULL, 0xb30d43f36b0d336aULL,
        0x908138468353418eULL, 0x764588140320cd76ULL,
        0xe1fdb97f79b80f5fULL, 0xafe399bfebefd0bbULL,
        0x3cbb086534a2d737ULL, 0xc760bfb1dd2e9c86ULL,
        0xc7e1243ef55a3244ULL, 0xd23c79dfae649be1ULL,
        0x2a448538cff3775aULL, 0xb500f1e0a9cf2f32ULL,
        0x5dcc4652551b93fdULL, 0xb1dfbe4c3edf28f8ULL,
        0xdeacc3cef3bb91ebULL, 0x886954747fb0670ULL,
        0x48ec2d6ca735f31eULL, 0x962811e1a91ffa03ULL,
        0x79365c0ba13d9600ULL, 0x752b064e1bd2fb7fULL,
        0x890ee3c477c68a7bULL, 0xfdcfa023a56e033ULL,
        0x1bb1f4abd53199e5ULL, 0x859f4d596bdc5ab6ULL,
        0x99af21f32a7bece2ULL, 0x8a73819dc1e3f7b2ULL,
        0x3da444a53e35f28fULL, 0xcee7e684308c0feULL,
        0x71b1408fd594001eULL, 0xf5da0a879300fbdfULL,
        0x6ea7b7380012b246ULL, 0x4c7b72cfbdc8228dULL,
        0xc8cb6646a653b933ULL, 0xd3fbb9ade2b9feb8ULL,
        0x533a8ecd379f0dd0ULL, 0x3ef5457b34378f04ULL,
        0x87651fdd3d0b159aULL, 0xeebd3a5eebb0f0ceULL,
        0x11531f89e3ed7d68ULL, 0x7ae5b58ceb78b16dULL,
        0xc9e3a581ea33efa7ULL, 0xac2a0f2b5ac06ef5ULL,
        0xf54f28b7504910bULL, 0x7807dfb069a3ca1bULL,
        0xfcc62e9e523d30b6ULL, 0x4fcf6d75fbd87500ULL,
        0x3b6681f84208bd2eULL, 0xc77ed69c61596008ULL,
        0x861ec31b1f586860ULL, 0x7822a364440369feULL,
        0x5865491d0e8164c7ULL, 0x7e403b716a89d86cULL,
        0x2e5534cdda8f34e3ULL, 0xfa06ca2439fbf5f4ULL,
        0x70c81b894b6e8fd4ULL, 0xd20063c34f995724ULL,
        0xf26e99c897926a10ULL, 0x126137c4c96007e7ULL,
        0x93bf4a74d3405908ULL, 0xee8010a30dd8d633ULL,
        0x818014eb8f776c5dULL, 0x5bb1192a88c962f5ULL,
        0x66a463bf9f5959dbULL, 0xd9778f81dd99d12aULL,
        0x1a9205e5b0bcae1eULL, 0x7ece5109ac03179dULL,
        0x44ff33a9c60a6b1dULL, 0x105367e212cd41deULL,
        0x6db30ef47c51d9f3ULL, 0xf3d23f14de1869a0ULL,
        0x2eddf69342aadc16ULL, 0x6ab93de87fcd1c94ULL,
        0xd4807458fcd1ee30ULL, 0x441315f3ec898103ULL,
        0x7b8bd586463d6d0aULL, 0x3ab11730d027a985ULL,
        0x5d3a6817a2eacd94ULL, 0x1d67fb36206c20f3ULL,
        0x1a207d5964a14c0cULL, 0x8fe013141f09dc6cULL,
        0x4f24bd886150fb39ULL, 0x4d0a8d6dacfdf7a1ULL,
        0xdf0b2967bd24662dULL, 0x5a0e7797f5634c26ULL,
        0x4dcfdcaa68188063ULL, 0xb5c531731b4ac1b0ULL,
        0x8a439a9a1fefa498ULL, 0x6d8d7d11f87d9c1ULL,
        0x2f05ace7c92a8152ULL, 0x41778cee5a4657aeULL,
        0x10d99d02f2d45395ULL, 0x4f700895026a66abULL,
        0x9e246db9b23e0aaeULL, 0x1374c77af85addffULL,
        0xbebc332ea23244f5ULL, 0xf520d066946ff875ULL,
        0xbb1576d812de78f4ULL, 0x3e86396a9f486867ULL,
        0x74f3883b42ce24acULL, 0x77ab67069d070b39ULL,
        0xe2a7aaf3b7d94835ULL, 0xe84ae13797205d5aULL,
        0xe2648dcbd5ddb6fcULL, 0x52759b397cffb297ULL,
        0xcdcb3254edadbc7dULL, 0x70491bf210d6636fULL,
        0xc26219b62028e978ULL, 0x694888c14a279af7ULL,
        0x621b4168ce1d47b4ULL, 0xa51293dc220b650dULL,
        0x19b7dcb18d059455ULL, 0x53986527ca72f73fULL,
        0xd6cd4cfaf7da433dULL, 0xf5224a7d886d4b8ULL,
        0x359aadf992622582ULL, 0x619f2607707edd64ULL,
        0xc779e8a8015579e2ULL, 0x4cf471e6066af1ceULL,
        0xfae3bf76eae98dfcULL, 0xec5dd514eb595eccULL,
        0xef37b0a96d0aa083ULL, 0x4af11c6f39a6efb2ULL,
        0x59d87c4203827a45ULL, 0x23d3b3f6ec47e3ceULL,
        0x226c13cbd1250359ULL, 0x461bb153206c4873ULL,
        0xa4db6d1e7e5149c9ULL, 0x777fb8c818e291d3ULL,
        0xb0d6aa720023a65dULL, 0xc05627e6e6dc7e5aULL,
        0x94d4ffa4be9b396eULL, 0x4555350c053c2784ULL,
        0x64d2a6987f2dc66dULL, 0x9e559eb47d85fd3bULL,
        0xe88998a1757985ULL, 0x94e721fa69ef3f64ULL,
        0xe986b3827b29384ULL, 0x588d0a2e71b7eb7cULL,
        0xff160c9a2fd263c3ULL, 0xf981c0baf19691beULL,
        0x286a031935c089ccULL};

constexpr U64 const *pieceRandTable = randTable;
constexpr U64 const *castlingRandTable = randTable + 768;
constexpr U64 const *epRandTable = randTable + 772;
constexpr U64 const *blackRand = randTable + 780;

#endif //BLACKCORE_CONSTANTS_H
