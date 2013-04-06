#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <assert.h>
#include "src/lib/vector.h"
#include "src/lib/util.h"
#include "src/lexer.h"

/* Algorithm outline:
 * - Read in entire file as a string.
 * - While we haven't reached the end of the string:
 *   - Go through each regex in tokenDefinitions.
 *   - If we find a regex that matches, get the token that the regex matches
 *     and add it to the list of lexmes.
 *   - If no regex matches, print out an error.
 *   - Maybe do some special error checking for numbers and identifiers.
 *   - Discard comment and whitespace tokens.
 * - Return list of lexemes.
 */

// Regexes that match each token type, along with the token type that they map
// to. The regexes are compiled with REG_EXTENDED (they are interpreted as
// POSIX extended regular expressions).
struct tokenDefinition tokenDefinitions[] = {

    // Whitespace
    {"\\s+", "whitespace"},
    // The comment regex is based off of http://ostermiller.org/findcomment.html
    // [*] matches a single * character, and looks slightely nicer than \\*
    // (\\* instead of \* because it needs to be escaped twice: once for the C
    // string and another time for the regex.)
    {"/[*]([^*]|[*]+[^*/])*[*]+/", "comment"},
    // Keywords
    // \\b = \b and matches the empty string, but only along a word boundary,
    // where words anything that contains [a-zA-Z0-9_].
    {"begin\\b", "begin"},
    {"while\\b", "while"},
    {"const\\b", "const"},
    {"write\\b", "write"},
    {"call\\b", "call"},
    {"then\\b", "then"},
    {"procedure\\b", "procedure"},
    {"read\\b", "read"},
    {"else\\b", "else"},
    {"odd\\b", "odd"},
    {"end\\b", "end"},
    {"int\\b", "int"},
    {"if\\b", "if"},
    {"do\\b", "do"},
    // Identifiers
    {"[a-zA-Z]\\w*", "identsym"},
    // Numbers
    {"[0-9]+", "numbersym"},
    // Special symbols
    {">=", ">="},
    {"<=", "<="},
    {"<>", "<>"},
    {":=", ":="},
    {"[+]", "+"},
    {"-", "-"},
    {"[*]", "*"},
    {"/", "/"},
    {"=", "="},
    {"<", "<"},
    {">", ">"},
    {"[(]", "("},
    {"[)]", ")"},
    {",", ","},
    {";", ";"},
    {"[.]", "."},
    // Indicates the end of token definitions.
    {NULL, NULL}

};

struct vector *readLexemes(char *source, struct vector *tokenDefinitions) {
    // Reset lexer errors.
    extern struct vector *lexerErrors;
    lexerErrors = NULL;

    initRegexes(tokenDefinitions);

    // Keep on trying to read lexemes and adding them to the list of lexemes
    // until we hit the end of the string.
    char *currentPosition = source;
    struct vector *lexemes = makeVector(struct lexeme);

    while (*currentPosition != '\0') {
        struct lexeme lexeme = readLexeme(currentPosition, tokenDefinitions);

        if (lexeme.token == NULL) {
            push(lexemes, lexeme);
            currentPosition += strlen(lexeme.token);
        } else {
            addLexerError(format("Unrecognized token starting at '%.10s...'.",
                        currentPosition));

            // If we hit an unrecognized token, just move ahead one character
            // and keep on trying.
            currentPosition += 1;
        }
    }

    return lexemes;
}

void initRegexes(struct vector *tokenDefinitions) {

    // For each token definition.
    forVectorPointers(tokenDefinitions, i, struct tokenDefinition, tokenDef,
        // Prepend ^ to all of the regexes so that it only matches tokens at the
        // current position in the source code.
        char *regexString = format("^%s", tokenDef->regexString);

        // Compile the regex.
        tokenDef->regex = (regex_t*)malloc(sizeof (regex_t));
        int error = regcomp(tokenDef->regex, regexString, REG_EXTENDED);

        // Check for errors.
        if (error != 0) {
            char *errorString = getRegexError(error, tokenDef->regex);
            addLexerError(format("Error compiling regex '%s': %s",
                        regexString, errorString));
            free(errorString);
        }

        free(regexString););

}

struct lexeme readLexeme(char *source, struct vector *tokenDefinitions) {
    // Try matching the regexes of every token definiton.
    forVector(tokenDefinitions, i, struct tokenDefinition, tokenDef,
        char *match = getMatch(tokenDef.regex, source);

        // Return the first successful match.
        if (match != NULL)
            return (struct lexeme){tokenDef.tokenType, match};);

    return (struct lexeme){NULL, NULL};
}

char *getMatch(regex_t *regex, char *string) {
    regmatch_t match;
    // Using the regex 'regex', search in the string 'string' for 1 match, and
    // store the result in regmatch_t match, and don't pass any special flags.
    int error = regexec(regex, string, 1, &match, 0);

    // The regex couldn't find any matches.
    if (error == REG_NOMATCH)
        return NULL;

    // Check for errors.
    if (error != 0) {

        char *errorString = getRegexError(error, regex);
        addLexerError(format("Error executing regex: %s", errorString));
        free(errorString);

    }

    // Extract and return the first match.
    int matchStart = match.rm_so;
    int matchEnd = match.rm_eo;
    int tokenLength = matchEnd - matchStart;

    if (matchStart >= 0 && tokenLength > 0) {
        char *result = substring(&string[matchStart], tokenLength);

        return result;
    }

    return NULL;
}

char *getRegexError(int error, regex_t *regex) {
    // Reserve 100 characters for the error message.
    int errorStringLength = 100;
    char *errorString = (char*)malloc(sizeof(char) * errorStringLength);
    regerror(error, regex, errorString, errorStringLength);

    return errorString;
}

struct vector *lexerErrors = NULL;

void addLexerError(char *message) {
    if (lexerErrors == NULL)
        lexerErrors = makeVector(char*);

    push(lexerErrors, message);
}

char *getLexerErrors() {
    if (lexerErrors == NULL)
        return NULL;

    return joinStrings(lexerErrors, "\n");
}

