#! /usr/bin/env bash

HERE=`dirname "$(readlink -f "$BASH_SOURCE")"`
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=${HERE}/rollup/crosstool.cmake ..
