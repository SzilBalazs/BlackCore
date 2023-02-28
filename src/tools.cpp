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

#include "tools.h"
#include "movegen.h"
#include "position.h"

#include <fstream>
#include <iostream>
#include <ostream>
#include <random>

constexpr int SHUFFLE_SIZE = 10000000;

void processPlain(const std::string filename) {
    std::ifstream file(filename);
    std::ofstream out("filtered.plain");

    initSearch();

    Position pos;
    std::random_device rd;
    std::mt19937 g(rd());

    int totalPositions = 0;

    while (true) {
        int filteredCapture = 0;
        int filteredChecks = 0;
        int filteredLowPly = 0;
        int filteredHighScores = 0;

        std::vector<std::string> lines;

        for (int i = 0; i < SHUFFLE_SIZE; i++) {
            std::string e, fen, move, score, ply, result;

            std::getline(file, fen);
            std::getline(file, move);
            std::getline(file, score);
            std::getline(file, ply);
            std::getline(file, result);

            if (file.eof()) {
                std::cout << "File end reached!" << std::endl;
                file.close();
                out.close();
                break;
            }

            fen = fen.substr(4);
            score = score.substr(6);
            result = result.substr(7);
            int movesPlayed = std::stoi(ply.substr(4));

            pos.loadPositionFromFen(fen);
            Square toSquare = stringToSquare(move.substr(7));

            std::getline(file, e);
            if (e != "e") {
                std::cout << "End not found: " << e << " " << std::endl;
                file.close();
                out.close();
                return;
            }

            if (movesPlayed <= 16) {
                filteredLowPly++;
                continue;
            }

            if (!pos.pieceAt(toSquare).isNull()) {
                filteredCapture++;
                continue;
            }

            if (getAttackers(pos, pos.pieces<KING>(pos.getSideToMove()).lsb()) != 0) {
                filteredChecks++;
                continue;
            }

            if (std::abs(std::stoi(score)) > 2000) {
                filteredHighScores++;
                continue;
            }

            std::string entry = fen + "<" + score + ">" + result + "\n";
            totalPositions++;
            lines.push_back(entry);
        }

        std::shuffle(lines.begin(), lines.end(), g);
        std::cout << "Total positions: " << totalPositions << " - Filtered captures: " << filteredCapture << " - Filtered early positions: " << filteredLowPly << " - Filtered checks: " << filteredChecks << " - Filtered out high score: " << filteredHighScores << std::endl;

        for (const std::string &line : lines) {
            out << line;
        }
        out.flush();
    }
}
