#include <stdio.h>

#include "test/lexer.c"
#include "test/parser.c"
#include "test/generator.c"

int main() {
    testLexer();
    testParser();
    testCodeGenerator();

    printf("All tests passed.\n");

    return 0;
}
