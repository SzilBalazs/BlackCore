<div align="center" style="padding-top: 50px">
    <img src="https://github.com/SzilBalazs/BlackCore/blob/020c7fa5be444b0053faba37ff4b25918acb6a3e/icon.png" alt="BlackCore logo by Graham Banks" width="200px" height="auto">
    <br>
    <br>
    <img src="https://img.shields.io/github/downloads/SzilBalazs/BlackCore/total?style=for-the-badge">
    <img src="https://img.shields.io/github/v/release/SzilBalazs/BlackCore?label=Latest%20release&style=for-the-badge">
    <br>
    <img src="https://img.shields.io/github/license/SzilBalazs/BlackCore?style=for-the-badge">
    <img src="https://img.shields.io/github/last-commit/SzilBalazs/BlackCore?style=for-the-badge">
</div>

## Overview

BlackCore is a UCI compatible c++ chess engine written from scratch.
Its alpha beta search uses various pruning techniques, powered by a neural network evaluation and a blazing fast
move generator.

### Playing strength - Last updated: 2022. 12. 30.

| Version   |   CCRL 2'+1" elo   |   CCRL 40/15 elo   | CEGT 40/4 elo |
|:----------|:------------------:|:------------------:|:-------------:|
| v4.0 4CPU | ~3200 (estimation) | ~3200 (estimation) |      N/A      |
| v4.0 1CPU | ~3100 (estimation) | ~3100 (estimation) |     3068      |
| v3.0 1CPU |        3069        |        3035        |     2941      |
| v2.0 1CPU |        N/A         |        2982        |      N/A      |
| v1.0 1CPU |        2134        |        N/A         |      N/A      |

## Installation

### Downloading prebuilt binary

You can download the latest release <a href="https://github.com/SzilBalazs/BlackCore/releases/latest">here</a> both for
Windows and Linux.
To select the right binary use the first instruction set that your CPU supports (doesn't crash), in the order of BMI2 ->
AVX2 -> popcnt

### Building from source (recommended)

After downloading the source, you can run the following commands, to build
a native binary.
This option gives the best performance.
**Please update your compiler before building!**

With any questions or problems feel free to create a github issue.

```
cd src
make clean build CXX=g++ ARCH=native
```

ARCH = popcnt/avx2/bmi2/native

CXX = the compiler of your choice (I recommend using g++, as it gives the best performance)

## Usage

BlackCore in itself is a command line program, and requires a UCI compatible
Chess GUI (like <a href="https://github.com/cutechess/cutechess">Cute Chess</a>
or <a href="http://www.playwitharena.de/">Arena</a>) for the best user experience.

### UCI Options

- **Hash** - The size of the Hash table in MB.
- **Threads** - The amount of threads that can be used in the search
- **Move Overhead** - The delay (in ms) between finding the best move and the GUI reacting to it. You may want to make
  this
  higher if you notice that the engine often runs out of time.


## Files

This project contains the following files:

- **README.md** the file that you are reading.
- **LICENSE** containing the license of this repository.
- **.github** folder contains automated GitHub workflows like building this project.
- **src** folder contains the source code of BlackCore

## Features

* UCI support
* Perft test
    * Up to ~240M nps (with NNUE accumulator disabled)
        * Intel i3-7100 3.9Ghz CPU
        * Single-threaded
        * Hashing disabled
* Benchmark
    * Fixed depth search on a set of custom positions
* 16 bit encoded moves
* Bitboard representation
* Engine
    * Search
        * Parameters tuned using <a href="https://github.com/dsekercioglu/weather-factory">weather
          factory</a>
        * Iterative deepening
        * Aspiration window
        * Alpha-Beta
            * Negamax
            * Transposition table
                * Entry aging
                * Bucket system
            * Principal variation search
                * Late move reduction/extension
                    * R = max(2, LMR_BASE + (ln(moveIndex) * ln(depth) / LMR_SCALE)));
                * Move count/late move pruning
                * Futility pruning
                * Singular extension
                * Check extension
            * Razoring
            * Reverse futility pruning
            * Null move pruning
        * Quintessence search
            * Stand-pat
            * Delta pruning
            * Static-exchange-evaluation pruning
        * Move ordering
            * Hash move
            * MVV-LVA and SEE
            * Killer, counter and history heuristics
            * History difference - killer move replacement
        * Multithreading support
          * Lazy SMP
    * Time management based on search stability
    * NNUE evaluation
        * Trained using <a href="https://github.com/SzilBalazs/CoreTrainer">CoreTrainer</a>
        * Training data was generated using <a href="https://github.com/jhonnold/berserk">Berserk</a> data
        * Support for AVX2 architecture for vectorized accumulator updates
        * Net embedded using incbin (for license see /src/incbin/UNLICENSE)

## Special thanks to

### <a href="https://www.chessprogramming.org/Main_Page">Chess Programming Wiki</a>

The Chess Programming Wiki is the greatest
resource for everybody who wants to be informed about the basics and the state-of-the-art technologies of chess
programming.

### <a href="https://github.com/AndyGrant/OpenBench">OpenBench</a> by <a href="https://github.com/AndyGrant">Andrew Grant</a>

OpenBench is a SPRT testing framework, used for the testing of different techniques in BlackCore

### <a href="https://github.com/dsekercioglu/weather-factory">Weather factory</a> by <a href="https://github.com/dsekercioglu">Pali</a>

Weather factory was used to train various parameters of BlackCore using
the <a href="https://www.chessprogramming.org/SPSA">
SPSA method</a>.

### <a href="https://github.com/jhonnold/berserk">Berserk</a> by <a href="https://github.com/jhonnold">Jay</a>

Berserk is a strong chess engine that generated the training data with the contribution of Sohail which was used in the
latest neural network and
crucial for the progress made in BlackCore.

### <a href="https://github.com/TheBlackPlague">Shaheryar Sohail</a>

Sohail (developer of <a href="https://github.com/TheBlackPlague/StockNemo">StockNemo</a>) helped me in countless
problems
regarding NNUE and I really can't thank him enough

### <a href="https://github.com/Disservin/Smallbrain">Smallbrain</a> by <a href="https://github.com/Disservin">Disservin</a>

Smallbrain is a great chess engine which helped me understand many important concepts, and thanks to Disservin for
giving me many great ideas how can I further improve my engine.

### <a href="https://github.com/official-stockfish/Stockfish">StockFish</a> by The StockFish team

Thanks to the StockFish team for making such a wonderful and an easy-to-read codebase, that inspired me to get into
chess programming in the first place.
