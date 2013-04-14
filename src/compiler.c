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
    // Print usage if the wrong number of arguments are given.
    if (!(argc == 2 || argc == 3)) {
        printf("Usage: %s <PL/0 source code filename> [<verbosity level>]\n", argv[0]);
        return 1;
    }

    // Set verbosity level.
    int verbosity = 0;
    if (argc == 3)
        verbosity = atoi(argv[2]);

    // Read in source code.
    char *sourceCode = readContents(argv[1]);
    if (sourceCode == NULL) {
        fprintf(stderr, "Error reading input file.\n");
        return 2;
    }

    // Print source code.
    if (verbosity >= 2)
        printf("Source code:\n%s\n", sourceCode);

    // Read tokens.
    struct vector *tokens = readPL0Tokens(sourceCode);
    if (tokens == NULL) {
        fprintf(stderr, "Error reading PL/0 tokens.\n");
        return 3;
    }

    // Print tokens.
    if (verbosity >= 3) {
        printf("Tokens:\n");
        forVector(tokens, i, struct token, token,
                printf("%s ", token.tokenType);
                if (strcmp(token.tokenType, "identifier-token") == 0
                    || strcmp(token.tokenType, "number-token") == 0)
                    printf("%s ", token.token););
        printf("\n\n");
    }

    // Parse tokens.
    struct parseTree tree = parsePL0Tokens(tokens);

    // Print parse tree.
    if (verbosity >= 4) {
        printf("Parse tree:\n");
        printParseTree(tree);
        printf("\n");
    }

    // Check for parser errors.
    if (isParseTreeError(tree)) {
        printf("Errors while parsing program:\n%s\n\n", getParserErrors());
        return 4;
    }

    // Generate code.
    struct vector *instructions = generatePL0(tree);

    // Check if the generator had errors.
    if (getGeneratorErrors() != NULL) {
        printf("The generator encountered errors:\n%s\n\n", getGeneratorErrors());
        if (instructions != NULL) {
            printf("This is what the generator was able to generate:\n");
            printInstructions(instructions, 1);
            printf("\n");
        }

        return 5;
    }

    if (verbosity >= 1)
        printf("No errors, program is syntactically correct.\n\n");

    // Print generated code.
    if (verbosity >= 1) {
        printf("Generated instructions:\n");
        // Print code with nice opcode names.
        printInstructions(instructions, 1);
    } else {
        // Print code suitable for the VM.
        printInstructions(instructions, 0);
    }

    return 0;
}
