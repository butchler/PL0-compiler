#ifndef PARSER_H
#define PARSER_H

#include "src/lib/vector.h"

// A parseTree is basically just a tree of strings.
struct parseTree {
    char *name;
    struct vector *children;
    int numTokens;
};

struct grammar {
    struct vector *rules;
};

struct rule {
    char *variable;
    struct vector *production;
};

struct parseTree parseProgram(struct vector *lexemes, struct grammar grammar);
struct parseTree parse(struct vector *lexemes, int index, char *currentVariable, struct grammar grammar);

int isTerminal(char *string);

// Returns the first child of the given parseTree that has the given name.
struct parseTree getChild(struct parseTree parent, char *childName);
// Returns the last child of the given parseTree that has the given name.
struct parseTree getLastChild(struct parseTree parent, char *childName);

#endif
