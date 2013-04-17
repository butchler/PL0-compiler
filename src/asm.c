#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "asm.h"
#include "lib/util.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        assert(argc >= 1);
        printf("Usage: %s <assembly source code file> [<verbosity level>]\n", argv[0]);
        return 1;
    }

    int verbosity = 0;
    if (argc >= 3)
        verbosity = atoi(argv[2]);

    // Read in source code.
    char *sourceCode = readContents(argv[1]);
    assert(sourceCode != NULL);

    // Print source code.
    if (verbosity >= 2)
        printf("Source code:\n%s\n", sourceCode);

    // Read tokens.
    struct vector *tokens = readAssembly(sourceCode);
    assert(tokens != NULL);

    // Print tokens.
    if (verbosity >= 3) {
        printf("Tokens:\n");
        forVector(tokens, i, struct token, token,
                printf("%s ", token.type);
                if (strcmp(token.type, "identifier-token") == 0
                    || strcmp(token.type, "number-token") == 0)
                    printf("%s ", token.token););
        printf("\n\n");
    }

    // Parse tokens.
    struct parseTree tree = parseAssembly(tokens);

    // Print parse tree.
    if (verbosity >= 4) {
        printf("Parse tree:\n");
        printParseTree(tree);
        printf("\n");
    }

    // Check for parser errors.
    if (isParseTreeError(tree)) {
        printf("Errors while parsing program:\n%s\n\n", getParserErrors());
        return 1;
    }

    // Generate code.
    char *instructions = assemble(tree);
    assert(instructions != NULL);

    // Print generated code.
    printf("%s\n", instructions);

    return 0;
}
