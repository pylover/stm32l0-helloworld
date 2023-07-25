#! /usr/bin/env bash

HERE=`dirname "$(readlink -f "$BASH_SOURCE")"`
mkdir -p ${HERE}/../build
cd ${HERE}/../build
cmake -DCMAKE_TOOLCHAIN_FILE=${HERE}/../rollup/crosstool.cmake ..
