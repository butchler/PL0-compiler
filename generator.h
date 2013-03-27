#ifndef GENERATOR_H
#define GENERATOR_H

#include "parser.h"
#include "vector.h"

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
