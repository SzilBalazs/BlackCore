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

#include <sstream>
#include <valarray>
#include "tuner.h"
#include "eval.h"

constexpr double K = 5;

double E(const std::vector<DataEntry> &data) {

    double error = 0;

    Position pos;

    for (const DataEntry &entry : data) {

        pos.loadPositionFromFen(entry.fen);

        auto score = double(eval(pos));

        double predicted = 1 / double(1 + pow(10, -K * score / 400));

        error += pow(entry.result - predicted, 2);

    }

    error /= double(data.size());

    return error;

}

void tune(const std::string &inputFile) {
    std::vector<DataEntry> trainingData;
    std::cout << "Loading training data..." << std::endl;
    std::ifstream f(inputFile);
    std::string line;
    while (std::getline(f, line)) {
        std::stringstream ss(line);
        std::string fen;
        int result;
        std::getline(ss, fen, ';');
        ss >> result;

        DataEntry entry = {fen, result == 0 ? 0 : (result == 1 ? 1 : 0.5)};

        trainingData.emplace_back(entry);
    }
    std::cout << trainingData.size() << " entry was loaded successfully!" << std::endl;

    // Local optimize algorithms
    bool improved = true;
    double bestE = E(trainingData);
    unsigned int iterationCount = 0;

    const unsigned int paramCnt = 10;
    Score *params[paramCnt] = {&PIECE_VALUES[PAWN].mg, &PIECE_VALUES[PAWN].eg,
                               &PIECE_VALUES[KNIGHT].mg, &PIECE_VALUES[KNIGHT].eg,
                               &PIECE_VALUES[BISHOP].mg, &PIECE_VALUES[BISHOP].eg,
                               &PIECE_VALUES[ROOK].mg, &PIECE_VALUES[ROOK].eg,
                               &PIECE_VALUES[QUEEN].mg, &PIECE_VALUES[QUEEN].eg,};

    while (improved) {
        improved = false;
        for (auto &param : params) {
            iterationCount++;

            std::cout << "Iteration " << iterationCount << ":\n - error = " << bestE << "\n - params = ";
            for (auto &p : params) {
                std::cout << (*p) << " ";
            }
            std::cout << std::endl;

            (*param)++;

            double newE = E(trainingData);

            if (newE < bestE) {
                bestE = newE;
                improved = true;
            } else {
                (*param) -= 2;
                newE = E(trainingData);
                if (newE < bestE) {
                    bestE = newE;
                    improved = true;
                }
            }

        }

    }
}