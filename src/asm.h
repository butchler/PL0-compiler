#ifndef ASM_H
#define ASM_H

#include "lib/vector.h"
#include "lib/parser.h"

// readAssembly is defined in asm-lexer.c, which is generated from asm.l by flex.
struct vector *readAssembly(char *sourceCode);

// Defined in asm-parser.c
struct parseTree parseAssembly(struct vector *tokens);

// Define in asm-generator.c
char *assemble(struct parseTree tree);

#endif
