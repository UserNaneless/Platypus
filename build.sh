#!/bin/bash

if [ -d "Build" ]; then
    rm -r Build
fi

BUILD_TYPE=Debug
BUILD=1

while [[ $# -gt 0 ]]; do
    case $1 in
        --Release)
            BUILD_TYPE=Release;
            shift
            ;;
        --Debug)
            BUILD_TYPE=Debug;
            shift
            ;;
        --switch)
            BUILD=0;
            shift
            ;;
        *)
            echo "Unknown parameter passed: $1"
            exit 1
            ;;
    esac
done

BUILD_DIR=Build/$BUILD_TYPE

mkdir -p $BUILD_DIR

cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_TOOLCHAIN_FILE=./cmake/gcc-arm-none-eabi.cmake 
ln -sf $BUILD_DIR/compile_commands.json compile_commands.json
ln -sf $BUILD_DIR/Platypus.elf Platypus.elf

if [ $BUILD -eq 1 ]; then
    make -C $BUILD_DIR
fi

