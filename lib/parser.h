#ifndef PARSER_H
#define PARSER_H

#include "lib/vector.h"
#include "lib/lexer.h"

extern struct generatorState;

// A parseTree is basically just a tree of strings.
struct parseTree {
    char *name;
    struct vector *children;
    int numTokens;   // The number of tokens that this parse tree represents.
    void (*generateFunction)(struct parseTree, struct generatorState);
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
    // Each rule can also have a generate function which is passed along to the
    // parse tree nodes that the rule matches.
    void (*generateFunction)(struct parseTree, struct generatorState);
};

// Parse the given tokens using the given grammar and start variable,
// returning a parse tree.
struct parseTree parse(struct vector *tokens, struct grammar grammar, char *startVariable);

// Returns a parse tree that indicates an error occurred, with the given error
// message as its name.
struct parseTree errorTree();
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
int hasChild(struct parseTree parent, char *childName);
struct parseTree getFirstChild(struct parseTree parent);
void printParseTree(struct parseTree tree);
// Recursively free a parse tree and all of its children.
void freeParseTree(struct parseTree tree);

// Functions for manipulating grammars
// ===================================
// Add a production rule to the given grammar. The production rule maps from
// variable -> productionString, where production string is a space-separated
// list of other variables and terminals that the variable should produce.
void addRule(struct grammar grammar, char *variable, char *productionString);

#endif
