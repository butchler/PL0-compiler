#include "src/pl0.h"
#include "src/lib/lexer.h"
#include "src/lib/parser.h"
#include "src/lib/vector.h"
#include "src/lib/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        assert(argc >= 1);
        printf("Usage: %s <PL/0 source code filename> [<verbosity level>]\n", argv[0]);
        return 1;
    }

    int verbose = 0;
    if (argc >= 3)
        verbose = atoi(argv[2]);

    // Read in source code.
    char *sourceCode = readContents(argv[1]);
    assert(sourceCode != NULL);

    // Print source code.
    if (verbose >= 2)
        printf("Source code:\n%s\n", sourceCode);

    // Read tokens.
    struct vector *tokens = readPL0Tokens(sourceCode);
    assert(tokens != NULL);

    // Print tokens.
    if (verbose >= 3) {
        printf("\nTokens:\n");
        forVector(tokens, i, struct token, token,
                printf("%s ", token.tokenType);
                if (strcmp(token.tokenType, "identifier-token") == 0
                    || strcmp(token.tokenType, "number-token") == 0)
                    printf("%s ", token.token););
        printf("\n\n");
    }

    // Parse tokens.
    struct parseTree tree = parsePL0Tokens(tokens);
    if (isParseTreeError(tree)) {
        printf("\nErrors while parsing program:\n%s\n", getParserErrors());
        return 1;
    } 

    // Print parse tree.
    if (verbose >= 4) {
        printf("Parse tree:\n");
        printParseTree(tree);

        printf("\n");
    }

    // Generate code.
    struct vector *instructions = generatePL0(tree);
    if (getGeneratorErrors() != NULL) {
        printf("The generator encountered errors:\n%s\n", getGeneratorErrors());
        if (instructions != NULL)
            printf("\nThis is what the generator was able to generate:\n");
        printInstructions(instructions, 1);

        return 1;
    }

    if (verbose >= 1)
        printf("\nNo errors, program is syntactically correct.\n");

    // Print generated code.
    assert(instructions != NULL);
    if (verbose >= 1) {
        printf("Generated instructions:\n");
        // Print code with nice opcode names.
        printInstructions(instructions, 1);
    } else {
        // Print code suitable for the VM.
        printInstructions(instructions, 0);
    }

    return 0;
}

