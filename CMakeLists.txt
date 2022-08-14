cmake_minimum_required(VERSION 3.23)
project(BlackCore)

set(CMAKE_CXX_FLAGS "-Ofast -pthread -march=native -flto -Wall")
set(CMAKE_CXX_STANDARD 20)

add_executable(BlackCore src/main.cpp src/move.cpp src/move.h src/constants.h src/utils.cpp src/utils.h src/position.cpp src/position.h src/bitboard.h src/movegen.cpp src/movegen.h src/bitboard.cpp src/search.cpp src/search.h src/eval.cpp src/eval.h src/move_ordering.cpp src/move_ordering.h src/tt.cpp src/tt.h)
