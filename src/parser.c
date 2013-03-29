#include "parser.h"
#include "lexer.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// Wrapper for sprintf that allocates the string for you.
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

int getTerminalType(char *terminal) {
    if (strcmp(terminal, "intsym") == 0) return INTSYM;
    if (strcmp(terminal, "semicolonsym") == 0) return SEMICOLONSYM;
    if (strcmp(terminal, "beginsym") == 0) return BEGINSYM;
    if (strcmp(terminal, "endsym") == 0) return ENDSYM;
    if (strcmp(terminal, "readsym") == 0) return READSYM;
    if (strcmp(terminal, "writesym") == 0) return WRITESYM;

    return -1;
}

struct parseTree parseProgram(struct vector *lexemes, struct grammar grammar) {
    struct parseTree result = parse(lexemes, 0, "program", grammar);
    if (result.numTokens == lexemes->length)
        return result;
    else
        return (struct parseTree){"Could not parse all tokens.", NULL, -1};
}

struct parseTree parse(struct vector *lexemes, int index, char *currentVariable, struct grammar grammar) {
    int startIndex = index;

    // Return a parseTree that indicates failure to parse.
    struct parseTree error(char *error) {
        return (struct parseTree){error, NULL, -1};
    }

    if (index >= lexemes->length)
        return error(format("Reached end of input while parsing variable '%s'.", currentVariable));

    struct lexeme currentLexeme = get(struct lexeme, lexemes, index);

    struct vector *children = makeVector(struct parseTree);

    // Special cases
    if (strcmp(currentVariable, "identifier") == 0) {
        if (currentLexeme.tokenType == IDENTSYM) {
            char *identifier = currentLexeme.token;
            pushLiteral(children, struct parseTree, {identifier, NULL});
            return (struct parseTree){"identifier", children, 1};
        } else {
            return error(format("Expected identifier, but got '%s'.", currentLexeme.token));
        }
    }
    if (strcmp(currentVariable, "number") == 0) {
        if (currentLexeme.tokenType == NUMBERSYM) {
            char *number = currentLexeme.token;
            pushLiteral(children, struct parseTree, {number, NULL});
            return (struct parseTree){"number", children, 1};
        } else {
            return error(format("Expected number, but got '%s'.", currentLexeme.token));
        }
    }

    int i;
    for (i = 0; i < grammar.rules->length; i++) {
        struct rule rule = get(struct rule, grammar.rules, i);
        if (strcmp(rule.variable, currentVariable) == 0) {
            // For each production for the current variable.

            // Reset state so we can try next production rule.
            freeVector(children);
            children = makeVector(struct parseTree);
            index = startIndex;

            int failed = 0;
            int j;
            for (j = 0; j < rule.production->length; j++) {
                // For each item in the production.
                if (index >= lexemes->length)
                    return error(format("Reached end of input while parsing variable '%s'.",
                                currentVariable));

                currentLexeme = get(struct lexeme, lexemes, index);

                char *varOrTerminal = get(char*, rule.production, j);
                if (isTerminal(varOrTerminal) || strcmp(varOrTerminal, "nothing") == 0) {
                    if (getTerminalType(varOrTerminal) == currentLexeme.tokenType) {
                        // Go to next token if this token matches the terminal.
                        index += 1;
                    } else if (strcmp(varOrTerminal, "nothing") == 0) {
                        // If we are matching the empty string, don't increase
                        // the index/go to the next token.
                    } else {
                        failed = 1;
                        break;
                    }
                } else {
                    struct parseTree child = parse(lexemes, index, varOrTerminal, grammar);
                    // A negative numTokens indicates an error occurred.
                    if (child.numTokens < 0) {
                        failed = 1;
                        break;
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

            if (!failed) {
                // Stop and return as soon as we find a production that matches.
                int numTokens = index - startIndex;
                return (struct parseTree){currentVariable, children, numTokens};
            }
        }
    }

    return error(format("No rules matched for variable %s", currentVariable));
}

int isTerminal(char *string) {
    int length = strlen(string);
    char *lastThreeChars = string + length - 3;

    if (strcmp(lastThreeChars, "sym") == 0)
        return 1;

    return 0;
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

