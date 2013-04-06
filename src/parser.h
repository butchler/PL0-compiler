#ifndef PARSER_H
#define PARSER_H

#include "src/lib/vector.h"
#include "src/lexer.h"

// A parseTree is basically just a tree of strings.
struct parseTree {
    char *name;
    struct vector *children;
    int numTokens;   // The number of tokens that this parse tree represents.
};

struct grammar {
    struct vector *rules;
};

struct rule {
    char *variable;
    struct vector *production;
};

// Parse the given lexemes using the given grammar and start variable,
// returning a parse tree.
struct parseTree parse(struct vector *lexemes, struct grammar grammar, char *startVariable);

// Returns a parse tree that indicates an error occurred, with the given error
// message as its name.
struct parseTree errorTree();
// Returns tree if the given tree is a tree that was produced by errorTree().
int isParseTreeError(struct parseTree tree);

// Converts "semicolonsym", "readsym", etc. into integers that represent the
// token type.
int getTokenType(char *token);

// Returns the first child of the given parseTree that has the given name.
struct parseTree getChild(struct parseTree parent, char *childName);
// Returns the last child of the given parseTree that has the given name.
struct parseTree getLastChild(struct parseTree parent, char *childName);
struct parseTree getFirstChild(struct parseTree parent);
int hasChild(struct parseTree parent, char *childName);
// Recursively free a parse tree and all of its children.
void freeParseTree(struct parseTree tree);

// Add a production rule to the given grammar. The production rule maps from
// variable -> productionString, where production string is a space-separated
// list of other variables and terminals that the variable should produce.
void addRule(struct grammar grammar, char *variable, char *productionString);

//char *setParserError(char *message);
void addParserError(char *message, int currentIndex);
char *getParserErrors();

#endif
