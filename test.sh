#!/bin/bash

if [ -f "test/test" ]; then
    rm test/test
fi

gcc -o test/test test/*.c test/lib/*.c src/*.c src/lib/*.c -I. -Isrc -Itest

if [ -f "test/test" ]; then
    ./test/test
fi

