#ifndef GENERATOR_H
#define GENERATOR_H

#include "parser.h"
#include "vector-types.h"

struct instruction {
    int opcode;
    int lexicalLevel;
    int modifier;
};

declareVector(instructionVector, struct instruction);

struct symbol {
    char *name;
    int level;
};

declareVector(symbolVector, struct symbol);

int getOpcode(char *instruction);
struct instruction makeInstruction(char *instruction, int lexicalLevel, int modifier);
struct parseTree getChild(struct parseTree parent, char *childName);
struct instructionVector *generateInstructions(struct parseTree tree);
struct instructionVector *generate(struct parseTree tree, struct symbolVector *symbols);

#endif
