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

#ifndef BLACKCORE_EVAL_H
#define BLACKCORE_EVAL_H

#include "constants.h"
#include "position.h"

Score eval(const Position &pos);

struct Value {
    Score mg=0;
    Score eg=0;

    inline Value operator+(Value a) const { return {mg+a.mg, eg+a.eg}; }

    inline Value operator-(Value a) const { return {mg-a.mg, eg-a.eg}; }

    inline Value operator*(int a) const { return {mg*a, eg*a}; }

    inline Value& operator+=(Value a) {
        mg += a.mg;
        eg += a.eg;
        return *this;
    }

    inline Value& operator+=(Score a) {
        mg += a;
        eg += a;
        return *this;
    }
};

constexpr Value PIECE_VALUES[6] = {{0,    0},
                                   {80,   140},
                                   {430,  480},
                                   {450,  530},
                                   {650,  850},
                                   {1300, 1700}};


constexpr Score TEMPO_SCORE = 10;

constexpr Value BISHOP_ATTACK_BONUS = {15, 5};

constexpr Value ROOK_OPEN_BONUS = {30, 20};
constexpr Value ROOK_HALF_BONUS = {10, 10};

constexpr Value wPawnTable[64] = {{  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0},
                                  { 15,   0}, { 10,   0}, {  10,  0}, {-20,   0}, {-20,   0}, {  10,  0}, {  10,  0}, { 15,   0},
                                  { 10,   0}, {  0,   0}, { -30,  0}, {  0,   0}, {  0,   0}, { -30,  0}, {  0,   0}, { 10,   0},
                                  {  0,   5}, {  0,  10}, {  0,  10}, { 30,  10}, { 30,  10}, {  0,  10}, {  0,  10}, {  0,   5},
                                  {  0,  15}, {  0,  20}, {  0,  20}, { 20,  20}, { 20,  20}, {  0,  20}, {  0,  20}, {  0,  15},
                                  {  5,  20}, { 10,  25}, { 10,  30}, { 10,  30}, { 10,  30}, { 10,  30}, { 10,  25}, {  5,  20},
                                  { 20,  50}, { 30,  55}, { 30,  60}, { 30,  60}, { 30,  60}, { 30,  60}, { 30,  55}, { 20,  50},
                                  {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}};

constexpr Value bPawnTable[64] = {
        {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0},
        { 20,  50}, { 30,  55}, { 30,  60}, { 30,  60}, { 30,  60}, { 30,  60}, { 30,  55}, { 20,  50},
        {  5,  20}, { 10,  25}, { 10,  30}, { 10,  30}, { 10,  30}, { 10,  30}, { 10,  25}, {  5,  20},
        {  0,  15}, {  0,  20}, {  0,  20}, { 20,  20}, { 20,  20}, {  0,  20}, {  0,  20}, {  0,  15},
        {  0,   5}, {  0,  10}, {  0,  10}, { 30,  10}, { 30,  10}, {  0,  10}, {  0,  10}, {  0,   5},
        { 10,   0}, {  0,   0}, { -30,  0}, {  0,   0}, {  0,   0}, { -30,  0}, {  0,   0}, { 10,   0},
        { 15,   0}, { 10,   0}, {  10,  0}, {-20,   0}, {-20,   0}, {  10,  0}, {  10,  0}, { 15,   0},
        {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}};

constexpr Score knightTable[64] = {-30, -20, -10, -10, -10, -10, -20, -30,
                                   -20, -20, 0, 0, 0, 0, -20, -20,
                                   -10, 0, 10, 15, 15, 10, 0, -10,
                                   -10, 0, 15, 20, 20, 15, 0, -10,
                                   -10, 0, 15, 20, 20, 15, 0, -10,
                                   -10, 0, 10, 15, 15, 10, 0, -10,
                                   -20, -20, 0, 0, 0, 0, -20, -20,
                                   -30, -20, -10, -10, -10, -10, -20, -30};

constexpr Value wKingTable[64] = {{ 30, -50}, { 30, -25}, {-10, -20}, {-20, -20}, {-20, -20}, {-10, -20}, { 30, -25}, { 30, -50},
                                  { 20, -25}, { 15, -10}, {-30,  -5}, {-30,   0}, {-30,   0}, {-30,  -5}, { 15, -10}, { 20, -25},
                                  {-30, -20}, {-30,  -5}, {-30,   0}, {-30,  10}, {-30,  10}, {-30,   0}, {-30,  -5}, {-30, -20},
                                  {-50, -20}, {-50,   0}, {-50,  10}, {-50,  30}, {-50,  30}, {-50,  10}, {-50,   0}, {-50, -20},
                                  {-50, -20}, {-50,   0}, {-50,  10}, {-50,  30}, {-50,  30}, {-50,  10}, {-50,   0}, {-50, -20},
                                  {-50, -20}, {-50,  -5}, {-50,   0}, {-50,  10}, {-50,  10}, {-50,   0}, {-50,  -5}, {-50, -20},
                                  {-50, -25}, {-50, -10}, {-50,  -5}, {-50,   0}, {-50,   0}, {-50,  -5}, {-50, -10}, {-50, -25},
                                  {-50, -50}, {-50, -25}, {-50, -20}, {-50, -20}, {-50, -20}, {-50, -20}, {-50, -25}, {-50, -50}};

constexpr Value bKingTable[64] = {{-50, -50}, {-50, -25}, {-50, -20}, {-50, -20}, {-50, -20}, {-50, -20}, {-50, -25}, {-50, -50},
                                 {-50, -25}, {-50, -10}, {-50,  -5}, {-50,   0}, {-50,   0}, {-50,  -5}, {-50, -10}, {-50, -25},
                                 {-50, -20}, {-50,  -5}, {-50,   0}, {-50,  10}, {-50,  10}, {-50,   0}, {-50,  -5}, {-50, -20},
                                 {-50, -20}, {-50,   0}, {-50,  10}, {-50,  30}, {-50,  30}, {-50,  10}, {-50,   0}, {-50, -20},
                                 {-50, -20}, {-50,   0}, {-50,  10}, {-50,  30}, {-50,  30}, {-50,  10}, {-50,   0}, {-50, -20},
                                 {-30, -20}, {-30,  -5}, {-30,   0}, {-30,  10}, {-30,  10}, {-30,   0}, {-30,  -5}, {-30, -20},
                                 { 20, -25}, { 15, -10}, {-30,  -5}, {-30,   0}, {-30,   0}, {-30,  -5}, { 15, -10}, { 20, -25},
                                 { 30, -50}, { 30, -25}, {-10, -20}, {-20, -20}, {-20, -20}, {-10, -20}, { 30, -25}, { 30, -50}};

#endif //BLACKCORE_EVAL_H
