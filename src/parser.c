#include "src/parser.h"
#include "src/lexer.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

struct parseTree parseProgram(struct vector *lexemes, struct grammar grammar) {
    struct parseTree result = parse(lexemes, 0, "program", grammar);
    if (result.numTokens == lexemes->length)
        return result;
    else
        return (struct parseTree){"Could not parse all tokens.", NULL, -1};
}

struct parseTree parse(struct vector *lexemes, int index, char *currentVariable, struct grammar grammar) {
    if (index >= lexemes->length)
        return errorTree(format("Reached end of input while parsing variable '%s'.", currentVariable));

    struct lexeme currentLexeme = get(struct lexeme, lexemes, index);

    // Special cases
    if (strcmp(currentVariable, "identifier") == 0)
        return parseValue(currentLexeme, IDENTSYM, "identifier");
    if (strcmp(currentVariable, "number") == 0)
        return parseValue(currentLexeme, NUMBERSYM, "number");

    struct parseTree result = errorTree(format("No rules found for variable %s", currentVariable));

    int i;
    for (i = 0; i < grammar.rules->length; i++) {
        struct rule rule = get(struct rule, grammar.rules, i);
        if (strcmp(rule.variable, currentVariable) == 0) {
            struct parseTree result = parseRule(rule, lexemes, index, currentVariable, grammar);
            int isError = result.numTokens < 0;
            if (!isError)
                return result;
        }
    }

    // If it gets to this point, that means that the result was an error, so
    // just return the error.
    return result;
}

struct parseTree parseValue(struct lexeme lexeme, int type, char *name) {
    if (lexeme.tokenType == type) {
        char *value = lexeme.token;
        struct vector *children = makeVector(struct parseTree);
        pushLiteral(children, struct parseTree, {value, NULL});
        return (struct parseTree){name, children, 1};
    } else {
        return errorTree(format("Expected %s, but got '%s'.", type, lexeme.token));
    }
}

struct parseTree parseRule(struct rule rule, struct vector *lexemes, int index,
        char *currentVariable, struct grammar grammar) {
    int startIndex = index;
    struct vector *children = makeVector(struct parseTree);

    int i;
    for (i = 0; i < rule.production->length; i++) {
        char *varOrTerminal = get(char*, rule.production, i);

        if (index >= lexemes->length)
            return errorTree(format("Reached end of input while parsing variable '%s'.",
                        currentVariable));

        struct lexeme currentLexeme = get(struct lexeme, lexemes, index);

        int tokenType = getTokenType(varOrTerminal);
        int isTerminal = (tokenType > 0);
        if (isTerminal) {
            if (tokenType == currentLexeme.tokenType) {
                // Go to next token if this token matches the terminal.
                index += 1;
            } else {
                return errorTree(format("Expected %s but got '%s'.",
                            varOrTerminal, currentLexeme.token));
            }
        } else if (!strcmp(varOrTerminal, "nothing") == 0) {
            struct parseTree child = parse(lexemes, index, varOrTerminal, grammar);
            int isError = (child.numTokens < 0);
            if (isError) {
                return errorTree(format("Expected %s starting at '%s'.",
                        varOrTerminal, currentLexeme.token));
            } else if (child.numTokens == 0) {
                // This should only happen if there's a rule that maps
                // directly to the empty string, i.e. variable ->
                // nothing. If that happens, don't add the child tree,
                // because it doesn't have anything in it and doesn't
                // contain any useful information.
            } else {
                // Add child and go to token after child's tokens.
                push(children, child);
                index += child.numTokens;
            }
        }
    }

    int numTokens = index - startIndex;
    return (struct parseTree){currentVariable, children, numTokens};
}

struct parseTree errorTree(char *error) {
    return (struct parseTree){error, NULL, -1};
}

int getTokenType(char *token) {
    if (strcmp(token, "intsym") == 0) return INTSYM;
    if (strcmp(token, "semicolonsym") == 0) return SEMICOLONSYM;
    if (strcmp(token, "beginsym") == 0) return BEGINSYM;
    if (strcmp(token, "endsym") == 0) return ENDSYM;
    if (strcmp(token, "readsym") == 0) return READSYM;
    if (strcmp(token, "writesym") == 0) return WRITESYM;

    return -1;
}

struct parseTree getChild(struct parseTree parent, char *childName) {
    int i;
    for (i = 0; i < parent.children->length; i++) {
        struct parseTree child = get(struct parseTree, parent.children, i);
        if (strcmp(child.name, childName) == 0)
            return child;
    }

    return (struct parseTree){NULL, NULL, -1};
}

struct parseTree getLastChild(struct parseTree parent, char *childName) {
    int i;
    for (i = parent.children->length - 1; i >= 0; i--) {
        struct parseTree child = get(struct parseTree, parent.children, i);
        if (strcmp(child.name, childName) == 0)
            return child;
    }

    return (struct parseTree){NULL, NULL, -1};
}

char *format(const char *fmt, ...) {
    int n;
    int size = 100;     /* Guess we need no more than 100 bytes */
    char *p, *np;
    va_list ap;

    if ((p = malloc(size)) == NULL)
        return NULL;

    while (1) {

        /* Try to print in the allocated space */

        va_start(ap, fmt);
        n = vsnprintf(p, size, fmt, ap);
        va_end(ap);

        /* Check error code */

        if (n < 0)
            return NULL;

        /* If that worked, return the string */

        if (n < size)
            return p;

        /* Else try again with more space */

        size = n + 1;       /* Precisely what is needed */

        if ((np = realloc (p, size)) == NULL) {
            free(p);
            return NULL;
        } else {
            p = np;
        }

    }
}

