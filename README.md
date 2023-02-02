<div align="center" style="padding-top: 50px">
    <img src="logo.png" alt="BlackCore logo by Midjourney" width="30%" style="padding: 10px">
    <br>
    <img src="https://img.shields.io/github/downloads/SzilBalazs/BlackCore/total?style=for-the-badge">
    <img src="https://img.shields.io/github/license/SzilBalazs/BlackCore?style=for-the-badge">
    <br>
    <img src="https://img.shields.io/github/v/release/SzilBalazs/BlackCore?label=Latest%20release&style=for-the-badge">
    <img src="https://img.shields.io/github/last-commit/SzilBalazs/BlackCore?style=for-the-badge">
</div>

# BlackCore

BlackCore is a c++ chess engine developed from scratch.
Its alpha beta search uses various pruning techniques, powered by a neural network evaluation and a blazing fast
move generator.

### Playing strength - Last updated: 2023. 2. 2.

| Version |   CCRL 2'+1" elo   |   CCRL 40/15 elo   |   CEGT 40/4 elo    | SPCC 3'+1" |
|:--------|:------------------:|:------------------:|:------------------:|:----------:|
| v5.1    | ~3300 (estimation) | ~3250 (estimation) | ~3200 (estimation) |            |
| v5.0    |                    |        3167        |        3143        |    3251    |
| v4.0    |        3182        |        3135        |        3068        |            |
| v3.0    |        3069        |        3035        |        2941        |            |
| v2.0    |                    |        2982        |                    |            |
| v1.0    |        2134        |                    |                    |            |

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
make clean build ARCH=native
```

ARCH = popcnt/avx2/bmi2/native

*If you wish to use another compiler than g++ set the CXX variable to for example clang. Warning: compatibility is not
guaranteed.*

## Usage

BlackCore in itself is a command line program, and requires a UCI compatible
Chess GUI (like <a href="https://github.com/cutechess/cutechess">Cute Chess</a>
or <a href="http://www.playwitharena.de/">Arena</a>) for the best user experience.

### UCI Options

- **Hash** - The size of the Hash table in MB.
- **Threads** - The amount of threads that can be used in the search
- **Move Overhead** - The delay (in ms) between finding the best move and the GUI reacting to it. You may want to make
  this higher if you notice that the engine often runs out of time.
- **SyzygyPath** (Optional) - The folder containing Syzygy tablebases.
- **EvalFile** (Optional) - The file containing the neural network which should be use. If it isn't found BlackCore will
  use the embedded network.

## Files

This project contains the following files:

- **README.md** the file that you are reading.
- **LICENSE** containing the license of this repository.
- **.github** folder contains automated GitHub workflows like building this project.
- **src** folder contains the source code of BlackCore
- **scripts** folder contains short scripts

## Features

* Hopefully an easy to read and well commented source
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
            * SEE
            * Killer, counter and history heuristics
        * Multithreading support
            * Lazy SMP
    * Time management based on search stability

## NNUE Ethics

As of v2.0 BlackCore uses neural networks for the evaluation of positions with support for AVX2 vectorization.
There is no need to worry about installing networks separately as they are embedded using incbin. Every net
was trained by me with training code which <a href="https://github.com/SzilBalazs/CoreTrainer">I wrote</a>.
Unfortunately I lack the hardware resources which are needed
to generate data, which means that external engines are used for that purpose. In my opinion this requires
transparency about the origins of the training data and acknowledgement of these engines.

### <a href="https://github.com/jhonnold/berserk">Berserk</a> by <a href="https://github.com/jhonnold">Jay Honnold</a>

Berserk is very strong chess engine that generated the training data with the contribution of Shaheryar Sohail which
were used continuously throughout v2.0-v5.0.

### <a href="https://lczero.org/">Leela Chess Zero</a> by The LC0 Team

LC0 uses a different approach for playing high level chess. It's powered by MCTS and acquired all of her chess knowledge
by selfplay.
Since v6.0-dev BlackCore uses data from <a href="https://storage.lczero.org/files/training_data/"> here</a>, which is
licensed under <a href="https://storage.lczero.org/files/training_data/LICENSE.txt">Open Database License</a>.

## Also thanks to...

### <a href="https://github.com/Disservin/Smallbrain">Smallbrain</a> by <a href="https://github.com/Disservin">Disservin</a>

Smallbrain is an awesome engine that helped me understand many important concepts, and a very special thanks to
Disservin for
giving me many great ideas how can I further improve BlackCore.

### <a href="https://www.chessprogramming.org/Main_Page">Chess Programming Wiki</a>

The Chess Programming Wiki is the greatest
resource for everybody who wants to be informed about the basics and the state-of-the-art technologies of chess
programming.

### <a href="https://github.com/AndyGrant/OpenBench">OpenBench</a> by <a href="https://github.com/AndyGrant">Andrew Grant</a>

OpenBench is a SPRT testing framework, used for the testing of different techniques in BlackCore

### <a href="https://github.com/TheBlackPlague">Shaheryar Sohail</a>

Sohail (developer of <a href="https://github.com/TheBlackPlague/StockNemo">StockNemo</a>) guided me through many
problems regarding NNUE and without .

### <a href="https://github.com/dsekercioglu/weather-factory">Weather factory</a> by <a href="https://github.com/dsekercioglu">Pali</a>

Weather factory was used to tune various parameters of BlackCore using
the <a href="https://www.chessprogramming.org/SPSA">
SPSA method</a>.

### <a href="https://github.com/official-stockfish/Stockfish">Stockfish</a> by The Stockfish team

Thanks to the Stockfish team for making such a wonderful and an easy-to-read codebase, that inspired me to get into
chess programming in the first place.
