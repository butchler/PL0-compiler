#ifndef PARSER_H
#define PARSER_H

#include "lib/vector.h"
#include "lib/lexer.h"

// A parseTree is basically just a tree of strings.
struct parseTree {
    char *name;
    struct vector *children;
    int numTokens;   // The number of tokens that this parse tree represents.
};

// A grammar holds the production rules of a context-free grammar.
struct grammar {
    struct vector *rules;
};

struct rule {
    // Each rule has a variable and a production rule consisting of several
    // variables or terminals.
    char *variable;
    struct vector *production;
};

// Parse the given tokens using the given grammar and start variable,
// returning a parse tree.
struct parseTree parse(struct vector *tokens, struct grammar grammar, char *startVariable);

// Returns a parse tree that indicates an error occurred, with the given error
// message as its name.
struct parseTree errorTree(char *name, struct vector *children);
// Returns tree if the given tree is a tree that was produced by errorTree().
int isParseTreeError(struct parseTree tree);

void addParserError(char *message, int currentIndex);
void clearParserErrors();
char *getParserErrors();

// Functions for manipulating parse trees
// ======================================
// Returns the first child of the given parseTree that has the given name.
struct parseTree getChild(struct parseTree parent, char *childName);
// Returns the last child of the given parseTree that has the given name.
struct parseTree getLastChild(struct parseTree parent, char *childName);
// Returns true if parent has a child with the given name.
int hasChild(struct parseTree parent, char *childName);
// Returns a list of all of the children of parent with the given name.
struct vector *getChildren(struct parseTree parent, char *childName);
struct parseTree getFirstChild(struct parseTree parent);

// Recursively print and free a parse tree and all of its children.
void printParseTree(struct parseTree tree);
void freeParseTree(struct parseTree tree);

// Functions for manipulating grammars
// ===================================
// Add a production rule to the given grammar. The production rule maps from
// variable -> productionString, where production string is a space-separated
// list of other variables and terminals that the variable should produce.
void addRule(struct grammar grammar, char *variable, char *productionString);

#endif
