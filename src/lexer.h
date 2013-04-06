#ifndef LEXER_H
#define LEXER_H

#include "src/lib/vector.h"
#include <regex.h>

#define MAX_IDENTIFIER_LENGTH 11
#define MAX_NUMBER_LENGTH 5

struct tokenDefinition {

    char *regexString;
    char *tokenType;
    regex_t *regex;

};


struct lexeme {

   char *tokenType;
   char *token;

};

// Given a string of source code and a vector of struct tokenDefinitions,
// return a vector of lexemes.
struct vector *readLexemes(char *source, struct vector *tokenDefinitions);

// Compile all of the regexes in the given tokenDefinitions.
void initRegexes(struct vector *tokenDefinitions);

// Try to read a single lexeme at the beginning of the given string of source
// code, returning an empty lexeme (i.e. (struct lexeme){NULL, NULL}) if there
// is no valid token at the beginning of the string.
struct lexeme readLexeme(char *source, struct vector *tokenDefinitions);

// Given a compiled regex and a string, return the first substring that matches
// the regex, or NULL if there is no match.
char *getMatch(regex_t *regex, char *string);

// Wrapper for regerror to get human readable error message if a regex function
// returns an error code.
char *getRegexError(int error, regex_t *regex);

void addLexerError(char *message);

#endif
