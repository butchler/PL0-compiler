#include <stdio.h>

#include "test/test.h"

int main() {

#include "test/lexer.c"
#include "test/parser.c"
#include "test/generator.c"

    printf("All tests passed.\n");

    return 0;
}
