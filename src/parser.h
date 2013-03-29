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

// Wrapper for parse that stries to parse the lexemes as a "program" variable.
// Use this instead of using parse directly.
struct parseTree parseProgram(struct vector *lexemes, struct grammar grammar);

// Parse the given lexemes, returning a parse tree.
struct parseTree parse(struct vector *lexemes, int index, char *currentVariable, struct grammar grammar);

// Used for parsing identifiers and numbers, which are kind of like terminals
// and kind of like variables. Returns a parse tree with the given name and the
// current token as the only child, if the current token is of the given type.
struct parseTree parseValue(struct lexeme lexeme, int type, char *name);

// Try to parse a single production rule for the given variable from the grammar.
struct parseTree parseRule(struct rule rule, struct vector *lexemes, int index,
        char *currentVariable, struct grammar grammar);

// Returns a parse tree that indicates an error occurred, with the given error
// message as its name.
struct parseTree errorTree(char *error);

// Converts "semicolonsym", "readsym", etc. into integers that represent the
// token type.
int getTokenType(char *token);

// Returns the first child of the given parseTree that has the given name.
struct parseTree getChild(struct parseTree parent, char *childName);
// Returns the last child of the given parseTree that has the given name.
struct parseTree getLastChild(struct parseTree parent, char *childName);

// Wrapper for sprintf that allocates the string for you. Copied from man 3
// printf. This function should really be in a more general utility function
// file.
char *format(const char *fmt, ...);

#endif
