#ifndef GENERATOR_H
#define GENERATOR_H

#include "parser.h"
#include "vector-types.h"

struct instruction {
    int opcode;
    char *opcodeName;
    int lexicalLevel;
    int modifier;
};

declareVector(instructionVector, struct instruction);

int getOpcode(char *instruction);
struct instruction makeInstruction(char *instruction, int lexicalLevel, int modifier);
struct instructionVector *generateInstructions(struct parseTree tree);
char *getGeneratorError();

#endif
