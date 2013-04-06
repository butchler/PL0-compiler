#ifndef PL0_H
#define PL0_H

// Returns a representation of a Context Free Gramar for the PL/0 language,
// suitable for use with parse() in parser.c.
struct grammar getPL0Grammar();

// Returns a list of token definitions for the tokens of the PL/0 language,
// suitable for use with readLexemes() in lexer.c.
struct vector *getPL0Tokens();

// Returns a new vector of lexemes that's the same as the given one but with
// all whitespace and comment tokens removed.
struct vector *removeWhitespaceAndComments(struct vector *lexeme);

#endif
