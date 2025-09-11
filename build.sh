#!/bin/bash

if [ -d "Build" ]; then
    rm -r Build
fi
mkdir Build
cd Build
if command -v cmake-stm >/dev/null 2>&1; then
    echo "cmake-stm found"
    cmake-stm ..
else
    echo "cmake-stm not found, using cmake"
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake 
fi
make
