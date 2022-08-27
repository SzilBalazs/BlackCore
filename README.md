<h1 align="center">BlackCore</h1>
<div align="center">
    <img src="https://github.com/SzilBalazs/BlackCore/actions/workflows/build.yml/badge.svg" alt="Test status">
</div>

## Overview

BlackCore is a UCI compatible c++ chess engine written from scratch, capable of completing high performance tasks.
Its alpha beta search uses various pruning techniques, powered by a handcrafted evaluation function and a blazing fast
move generator.

## Files

This project contains the following files:

- **README.md** the file that you are reading.
- **LICENSE** containing the license of this repository.
- **.github** folder contains automated GitHub workflows like building this project.
- **src** folder contains the source code of BlackCore

## Features

* UCI support
* Perft test
    * Up to ~230M nps
        * Intel i3-7100 3.9Ghz CPU
        * Single-threaded
        * Hashing disabled
* Benchmark
    * Fixed depth search on a set of custom positions
* 16 bit encoded moves
* Bitboard representation
* Engine
    * Search
        * Iterative deepening
        * Alpha-Beta
            * Negamax
            * Transposition table
                * Cut-offs
                * Entry aging
                * Bucket system
                    * 1 always replace
                    * 1 depth preferred
            * Principal variation search
                * Late move pruning
                    * R = LMR_BASE + (sqrt(index - 1) + sqrt(depth - 1)) / LMR_SCALE
            * Razoring
                * Dropping into qsearch at frontier nodes
            * Reverse futility pruning
                * With improving detection
            * Null move pruning
                * Reduction depends on depth searched
        * Quintessence search
            * Stand-pat
            * Delta pruning
            * Static-exchange-evaluation pruning
        * Move ordering
            * Hash move
            * Promotions
            * Under promotions
            * Captures
                * MVV-LVA
                * SEE - currently disabled
            * Quiet moves
                * Killer heuristic
                * History heuristic
        * Fast repetition detection
    * Time management
        * Sudden death
        * Increment per move
        * Move-time
        * Moves to go support
    * Handcrafted evaluation
        * Tapered eval
            * Mid-game and end-game
        * Material balance
        * Pawn structure
            * Double pawns
            * Isolated pawns
            * Passed pawns
        * King safety
            * Pawn king shield
            * Bonus for castled king
            * Trapped rooks restricted by uncastled king
        * Knight mobility
            * Restricted by enemy pawns
        * Bishop mobility
            * Depending on the pawn structure
        * Rooks
            * Mobility
            * Bonuses for open and half open files
        * Tempo

## Installation

### Building from source

After downloading the sources (preferably the source of the latest release) you can run the following commands, to build
a native binary.
BlackCore uses c++20 standard, so older versions of compilers might not work.

```
cd src
make clean build CXX=g++-11 ARCH=native
```

ARCH = popcnt/modern/bmi2/native

CXX = the compiler of your choice (I recommend using g++, as it gives the best performance)

### Downloading prebuilt binary

You can download the latest release <a href="https://github.com/SzilBalazs/BlackCore/releases/latest">here</a> both for
Windows and Linux.
To select the right binary, choose the bmi2 build if you have a fairly new CPU, otherwise you can use the modern build
or in case of an older
processor use the popcnt build. Only 64 bits CPUs with popcnt are supported at the moment.

## Usage

BlackCore in itself is a command line program, and requires a UCI compatible
Chess GUI (like <a href="https://github.com/cutechess/cutechess">Cute Chess</a>
or <a href="http://www.playwitharena.de/">Arena</a>) for the best user experience.

### UCI Options

- **Hash** - The size of the Hash table in MB.
- **Threads** - Currently BlackCore only supports single threaded search, but this will probably change in the future.
- **Move Overhead** - The delay (in ms) between finding the best move and the GUI reacting to it. You may want to make
  this
  higher if you notice that the engine often runs out of time.

## Big thanks to

### <a href="https://www.chessprogramming.org/Main_Page">Chess Programming Wiki</a>

The Chess Programming Wiki is the greatest
resource for everybody who wants to be informed about the basics and the state-of-the-art technologies of chess
programming.

### <a href="https://github.com/AndyGrant/OpenBench">OpenBench</a> by Andrew Grant

OpenBench is an usefull SPRT testing framework, which contributed
to the development of BlackCore substantially.

### <a href="https://github.com/official-stockfish/Stockfish">StockFish</a> by The StockFish team

Thanks to the StockFish team for making such a wonderful and an easy-to-read codebase, that inspired me to get into
chess programming in the first place.

### <a href="https://github.com/Disservin/Smallbrain">Smallbrain</a> by <a href="https://github.com/Disservin">Disservin</a>

Smallbrain is a great chess engine which helped me understand many important concepts, and thanks to Disservin for
giving me many great ideas how can I further improve my engine.

