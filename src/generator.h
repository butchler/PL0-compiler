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

struct symbol {
    char *name;
    int level;     // The lexical level of the symbol.
    int address;   // The address of the symbol on the stack, in it's lexical level.
};

// Used in the generate function to keep track of the current state.
struct generatorState {
    struct vector *symbols;
    int currentAddress;   // The current code address.
    int currentLevel;     // The current lexical level.
};

struct vector *generateInstructions(struct parseTree tree);
struct vector *generate(struct parseTree tree, struct generatorState state);
struct vector *generate_program(struct parseTree tree, struct generatorState state);
struct vector *generate_block(struct parseTree tree, struct generatorState state);
struct vector *generate_varDeclaration(struct parseTree tree, struct generatorState state);
struct vector *generate_statement(struct parseTree tree, struct generatorState state);
struct vector *generate_statements(struct parseTree tree, struct generatorState state);
struct vector *generate_beginBlock(struct parseTree tree, struct generatorState state);
struct vector *generate_readStatement(struct parseTree tree, struct generatorState state);
struct vector *generate_writeStatement(struct parseTree tree, struct generatorState state);
struct vector *generate_ifStatement(struct parseTree tree, struct generatorState state);
struct vector *generate_expression(struct parseTree tree, struct generatorState state);
struct vector *generate_relationalOperator(struct parseTree tree, struct generatorState state);

int getOpcode(char *instruction);
struct instruction makeInstruction(char *instruction, int lexicalLevel, int modifier);

struct symbol addSymbol(struct generatorState state, char *name);
struct symbol getSymbol(struct generatorState state, char *name);

void setGeneratorError(char *errorMessage);
char *getGeneratorError();

#endif
