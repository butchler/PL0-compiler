#ifndef TEST_PARSER_H
#define TEST_PARSER_H

#include "src/parser.h"

// This file holds functions used to test the parser.

// Awesome alias for generateParseTree so that you don't have to type a bunch
// of quotes or \'s to make a multiline string.
#define pt(forms) generateParseTree("(" #forms ")")

// Generates a parseTree struct from a string of lisp-like forms that
// represent the structure of the parse tree.
struct parseTree generateParseTree(char *forms);

// Takes a string containing a lisp-like form, and returns a vector of strings
// representing the items in the forms. For example, (a (b c) d) becomes
// ["a", "(b c)", "d"].
struct vector *formToVector(char *form);

// Return true if the two parse trees are equivalent, false otherwise.
int parseTreesEqual(struct parseTree x, struct parseTree y);

#endif
