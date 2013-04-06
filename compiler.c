#include "src/pl0.h"
#include "src/lexer.h"
#include "src/parser.h"
#include "src/generator.h"
#include "src/lib/vector.h"
#include "src/lib/util.h"
#include "test/lib/parser.h"
#include "test/lib/generator.h"
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

    // Initialize compiler.
    struct grammar grammar = getPL0Grammar();

    // Read in source code.
    char *sourceCode = readContents(argv[1]);
    assert(sourceCode != NULL);

    // Print source code.
    if (verbose >= 2)
        printf("Source code:\n%s\n", sourceCode);

    struct vector *tokenTypes = getPL0Tokens();

    // Read tokens.
    struct vector *lexemes = readLexemes(sourceCode, tokenTypes);
    assert(lexemes != NULL);
    freeVector(tokenTypes);

    // Check for errors while reading tokens.
    if (getLexerErrors() != NULL)
        printf("\nErrors while reading tokens:\n%s\n", getLexerErrors());

    // Remove whitespace and comment tokens.
    // TODO: Free original lexemes, before whitespace and comments removal.
    lexemes = removeWhitespaceAndComments(lexemes);

    // Print tokens.
    if (verbose >= 3) {
        printf("\nTokens:\n");
        forVector(lexemes, i, struct lexeme, lexeme,
                printf("%s ", lexeme.tokenType);
                if (strcmp(lexeme.tokenType, "identifier-token") == 0
                    || strcmp(lexeme.tokenType, "number-token") == 0)
                    printf("%s ", lexeme.token););
        printf("\n\n");
    }

    // Parse tokens.
    struct parseTree tree = parse(lexemes, grammar, "program");
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
    struct vector *instructions = generateInstructions(tree);
    if (generatorHasErrors()) {
        printf("The generator encountered errors:\n");
        printGeneratorErrors();
        if (instructions != NULL)
            printf("\nThis is what the generator was able to generate:\n");
        printInstructions(instructions);

        return 1;
    }

    if (verbose >= 1)
        printf("\nNo errors, program is syntactically correct.\n");

    // Print generated code.
    assert(instructions != NULL);
    if (verbose >= 1) {
        printf("Generated instructions:\n");
        // Print code with nice opcode names.
        printInstructions(instructions);
    } else {
        // Print code suitable for the VM.
        forVector(instructions, i, struct instruction, instruction,
                printf("%d %d %d\n",
                    instruction.opcode,
                    instruction.lexicalLevel,
                    instruction.modifier););
    }

    return 0;
}

