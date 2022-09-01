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
#include <iomanip>
#include <valarray>
#include "tuner.h"
#include "eval.h"

constexpr double K = 1.5;

double E(const std::vector<DataEntry> &data) {

    double error = 0;

    Position pos;

    for (const DataEntry &entry : data) {

        pos.loadPositionFromRawState(entry.pos);

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
    Position position;
    while (std::getline(f, line)) {
        std::stringstream ss(line);
        std::string fen;
        int result;
        std::getline(ss, fen, ';');
        ss >> result;
        position.loadPositionFromFen(fen);
        DataEntry entry = {position.getRawState(), result == 0 ? 0 : (result == 1 ? 1 : 0.5)};

        trainingData.emplace_back(entry);
    }
    std::cout << trainingData.size() << " entry was loaded successfully!" << std::endl;

    // Local optimize algorithms
    bool improved = true;
    double bestE = E(trainingData);
    unsigned int iterationCount = 0;

    const unsigned int paramCnt = 768;

    while (improved) {
        improved = false;
        for (unsigned int idx = 0; idx < paramCnt; idx++) {
            iterationCount++;

            unsigned int index = idx;

            auto pieceType = static_cast<PieceType>(int(index / 128));
            index %= 128;

            bool isMgScore = 1 - int(index / 64);
            index %= 64;

            auto whiteSquare = static_cast<Square>(index);

            unsigned int rank = squareToRank(whiteSquare), file = squareToFile(whiteSquare);
            auto blackSquare = Square((7 - rank) * 8 + file);


            if (isMgScore) {
                PSQT[WHITE][pieceType][whiteSquare].mg += 1;
                PSQT[BLACK][pieceType][blackSquare].mg += 1;
            } else {
                PSQT[WHITE][pieceType][whiteSquare].eg += 1;
                PSQT[BLACK][pieceType][blackSquare].eg += 1;
            }

            double newE = E(trainingData);

            if (newE < bestE) {
                bestE = newE;
                improved = true;
            } else {

                if (isMgScore) {
                    PSQT[WHITE][pieceType][whiteSquare].mg -= 2;
                    PSQT[BLACK][pieceType][blackSquare].mg += 2;
                } else {
                    PSQT[WHITE][pieceType][whiteSquare].eg -= 2;
                    PSQT[BLACK][pieceType][blackSquare].eg -= 2;
                }


                newE = E(trainingData);
                if (newE < bestE) {
                    bestE = newE;
                    improved = true;
                }
            }

            std::cout << "Iteration " << iterationCount << ":\n - error = " << bestE << "\n - last param = "
                      << (int) pieceType << "(type) " << formatSquare(whiteSquare) << " "
                      << (isMgScore ? "midgame" : "endgame")
                      << std::endl;

            if (iterationCount % 20 == 0) {
                std::ofstream params("params.txt");
                params << "Iteration = " << iterationCount << "\n";

                for (unsigned int type = 0; type < 6; type++) {
                    params << "\nconstexpr Score " << typeToString(static_cast<PieceType>(type))
                           << "MgPSQT = {\n\t";
                    for (Square sq = A1; sq < 64; sq += 1) {
                        params << std::setw(4) << PSQT[BLACK][type][sq].mg << ", ";
                        if (squareToFile(sq) == 7) {
                            params << "\n";
                            if (squareToRank(sq) != 7) {
                                params << "\t";
                            }
                        }
                    }
                    params << "};\n";

                    params << "\nconstexpr Score " << typeToString(static_cast<PieceType>(type))
                           << "EgPSQT = {\n\t";
                    for (Square sq = A1; sq < 64; sq += 1) {
                        params << std::setw(4) << PSQT[BLACK][type][sq].eg << ", ";
                        if (squareToFile(sq) == 7) {
                            params << "\n";
                            if (squareToRank(sq) != 7) {
                                params << "\t";
                            }
                        }

                    }
                    params << "};\n";
                }

                params.close();
            }
        }

    }
}