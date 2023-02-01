#!/bin/bash
#
# BlackCore is a chess engine
# Copyright (c) 2023 SzilBalazs
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

cd ..
mkdir release
cd src
for ARCH in 'popcnt' 'avx2' 'bmi2'
do
  make clean
  make -j 6 CXX=x86_64-w64-mingw32-g++ EXE=../release/BlackCore-$ARCH-win.exe ARCH=$ARCH
  make clean
  make -j 6 CXX=g++ EXE=../release/BlackCore-$ARCH-linux ARCH=$ARCH
done