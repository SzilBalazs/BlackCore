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

if [ $# -eq 0 ]; then
    echo "Please provide a branch name! Example usage: \"./update.sh tcec\""
    exit 1
fi

rm -rf BlackCore
git clone https://github.com/SzilBalazs/BlackCore --branch $1
g++ --version
make -C BlackCore/src -j 6 CXX=g++ ARCH=native EXE=BlackCore-$1
cp BlackCore/src/BlackCore-$1 ./
EXE=BlackCore-$1
./BlackCore-$1 bench
rm -rf BlackCore
