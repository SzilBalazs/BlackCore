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

#include "tuner.h"
#include "eval.h"
#include <iomanip>
#include <sstream>
#include <valarray>

double K = 0.2;

double E(const std::vector<DataEntry> &data) {

    double error = 0;

    Position pos;

    for (const DataEntry &entry : data) {

        pos.loadPositionFromRawState(entry.pos);

        auto score = double(eval(pos));

        double predicted = 1 / double(1 + pow(10, -K * score / 400));

        error += (entry.result - predicted) * (entry.result - predicted);
    }

    error /= double(data.size());

    return error;
}

void saveResults(const unsigned int paramCnt, EvalParameter *evalParameters) {
#ifdef TUNE_
    std::ofstream params("params.txt");

    for (unsigned int i = 0; i < paramCnt; i++) {
        params << "\nconstexpr Value "
               << evalParameters[i].name << " = {"
               << evalParameters[i].mgScore << ", "
               << evalParameters[i].egScore << "};\n";
    }

    params << "\n";

    for (unsigned int type = 0; type < 6; type++) {
        params << "\nconstexpr Score " << typeToString(static_cast<PieceType>(type))
               << "MgPSQT[64] = {\n\t";
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
               << "EgPSQT[64] = {\n\t";
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
#endif
}

void tune(const std::string &inputFile) {
#ifdef TUNE_
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
    std::cout << trainingData.size() << " entry was loaded successfully!\nOptimizing K..." << std::endl;

    double bestK = 0;
    double bestError = 2;
    while (K <= 2) {
        double newE = E(trainingData);
        std::cout << "K = " << K << " E = " << newE << std::endl;
        if (newE < bestError) {
            bestError = newE;
            bestK = K;
        }
        K += 0.1;
    }

    K = bestK;
    std::cout << "Best K = " << K << " with an error of " << bestError << std::endl;

    // Local optimize algorithms
    bool improved = true;
    double bestE = E(trainingData);
    unsigned int iterationCount = 0;

    const unsigned int PSQTparamCnt = 768;
    const unsigned int paramCnt = 0;

    EvalParameter params[paramCnt] = {};


    while (improved) {
        improved = false;

        // Tuning the PSQT table

        for (unsigned int idx = 0; idx < PSQTparamCnt; idx++) {
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
            int change = 0;
            if (newE < bestE) {
                bestE = newE;
                improved = true;
                change = 1;
            } else {

                if (isMgScore) {
                    PSQT[WHITE][pieceType][whiteSquare].mg -= 2;
                    PSQT[BLACK][pieceType][blackSquare].mg -= 2;
                } else {
                    PSQT[WHITE][pieceType][whiteSquare].eg -= 2;
                    PSQT[BLACK][pieceType][blackSquare].eg -= 2;
                }


                newE = E(trainingData);
                if (newE < bestE) {
                    bestE = newE;
                    improved = true;
                    change = -1;
                } else {
                    if (isMgScore) {
                        PSQT[WHITE][pieceType][whiteSquare].mg += 1;
                        PSQT[BLACK][pieceType][blackSquare].mg += 1;
                    } else {
                        PSQT[WHITE][pieceType][whiteSquare].eg += 1;
                        PSQT[BLACK][pieceType][blackSquare].eg += 1;
                    }
                }
            }

            std::cout << "Iteration " << iterationCount << ":\n - error = " << bestE << "\n - previous param = "
                      << (int) pieceType << " " << formatSquare(whiteSquare)
                      << (isMgScore ? " midgame (" : " endgame (") << change << ")" << std::endl;

            if (iterationCount % 10 == 0) {
                saveResults(paramCnt, params);
            }
        }

        // Tuning other eval params
        for (auto &param : params) {
            // Tuning the midgame part of the value
            iterationCount++;

            param.mgScore += 1;
            double newE = E(trainingData);
            int change = 0;

            if (newE < bestE) {
                bestE = newE;
                improved = true;
                change = 1;
            } else {
                param.mgScore -= 2;
                newE = E(trainingData);

                if (newE < bestE) {
                    bestE = newE;
                    improved = true;
                    change = -1;
                } else {
                    param.mgScore += 1;
                }
            }

            std::cout << "Iteration " << iterationCount << ":\n - error = " << bestE << "\n - previous param = "
                      << param.name << " midgame (" << change << ")" << std::endl;


            // Tuning the endgame part of the value
            iterationCount++;

            param.egScore += 1;
            newE = E(trainingData);
            change = 0;

            if (newE < bestE) {
                bestE = newE;
                improved = true;
                change = 1;
            } else {
                param.egScore -= 2;
                newE = E(trainingData);

                if (newE < bestE) {
                    bestE = newE;
                    improved = true;
                    change = -1;
                } else {
                    param.egScore += 1;
                }
            }

            std::cout << "Iteration " << iterationCount << ":\n - error = " << bestE << "\n - previous param = "
                      << param.name << " endgame (" << change << ")" << std::endl;

            if (iterationCount % 10 == 0) {
                saveResults(paramCnt, params);
            }
        }
    }
#endif
}
