#!/bin/bash
# Recompile and run test/test.c

if [ -f "test/test" ]; then rm test/test; fi

gcc -g -o test/test test/test.c lib/*.c test/lib/*.c -I. -lfl

if [ -f "test/test" ]; then ./test/test; fi

