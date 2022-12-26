#!/bin/bash
cd ..
mkdir release
cd src
for ARCH in 'popcnt' 'avx2' 'bmi2'
do
  make clean
  make -j CXX=x86_64-w64-mingw32-g++-posix EXE=../release/BlackCore-$ARCH-win.exe ARCH=$ARCH
  make clean
  make -j CXX=g++ EXE=../release/BlackCore-$ARCH-linux ARCH=$ARCH
done