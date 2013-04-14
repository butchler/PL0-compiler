#!/bin/bash
# Recompile and run test/test.c

if [ -f "test/test" ]; then rm test/test; fi

gcc -g -o test/test -I.\
    test/test.c\
    test/lib/*.c\
    src/pl0-lexer.c -lfl\
    src/pl0-parser.c\
    src/pl0-generator.c\
    src/lib/*.c\

if [ -f "test/test" ]; then ./test/test; fi

