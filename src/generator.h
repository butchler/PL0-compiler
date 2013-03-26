#ifndef GENERATOR_H
#define GENERATOR_H

// Include parse for the parse tree type
// which is referenced in this file
#include "src/parser.h"
#include "src/lib/vector.h"

struct instruction {
    int opcode;
    char *opcodeName;
    int lexicalLevel;
    int modifier;
};

int getOpcode(char *instruction);
struct instruction makeInstruction(char *instruction, int lexicalLevel, int modifier);
struct vector *generateInstructions(struct parseTree tree);
char *getGeneratorError();

#endif
