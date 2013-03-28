#include "parser.h"
#include "lexer.h"
#include <string.h>

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
        return (struct parseTree){"Could not parse all tokens.", NULL};
}

struct parseTree parse(struct vector *lexemes, int index, char *currentVariable, struct grammar grammar) {
    int startIndex = index;

    // Return a parseTree that indicates failure to parse.
    /*struct parseTree reject() {
        return (struct parseTree){NULL, NULL};
    }*/
    struct parseTree reject(char *error) {
        return (struct parseTree){error, NULL};
    }

    if (index >= lexemes->length)
        return reject("index >= lexemex->length");

    struct lexeme currentLexeme = get(struct lexeme, lexemes, index);

    struct vector *children = makeVector(struct parseTree);

    // Special cases
    if (strcmp(currentVariable, "identifier") == 0) {
        if (currentLexeme.tokenType == IDENTSYM) {
            char *identifier = currentLexeme.token;
            pushLiteral(children, struct parseTree, {identifier, NULL});
            return (struct parseTree){"identifier", children, 1};
        } else {
            return reject("Invalid identifier");
        }
    }
    if (strcmp(currentVariable, "number") == 0) {
        if (currentLexeme.tokenType == IDENTSYM) {
            char *number = currentLexeme.token;
            pushLiteral(children, struct parseTree, {number, NULL});
            return (struct parseTree){"number", children, 1};
        } else {
            return reject("Invalid number");
        }
    }

    int i;
    for (i = 0; i < grammar.rules->length; i++) {
        struct rule rule = get(struct rule, grammar.rules, i);
        if (strcmp(rule.variable, currentVariable) == 0) {
            // For each production for the current variable.
            // Reset state.
            freeVector(children);
            children = makeVector(struct parseTree);
            index = startIndex;
            int failed = 0;
            int j;
            for (j = 0; j < rule.production->length; j++) {
                // For each item in the production.
                if (index >= lexemes->length)
                    return reject("index >= lexemes->length #2");

                currentLexeme = get(struct lexeme, lexemes, index);

                char *varOrTerminal = get(char*, rule.production, j);
                if (isTerminal(varOrTerminal)) {
                    if (getTerminalType(varOrTerminal) == currentLexeme.tokenType)
                        // Go to next token if this token matches the terminal.
                        index += 1;
                    else {
                        // TODO: How to give good error messages?
                        //return reject("Terminal not matched.");
                        failed = 1;
                        break;
                    }
                } else {
                    struct parseTree child = parse(lexemes, index, varOrTerminal, grammar);
                    if (child.name == NULL) {
                        // TODO: How to give good error messages?
                        //return reject(child.name);
                        failed = 1;
                        break;
                    }
                    // Add child and go to token after child's tokens.
                    push(children, child);
                    index += child.numTokens;
                }
            }

            if (!failed)
                // Stop and return as soon as we find a production that
                // matches.
                break;
        }
    }

    int numTokens = index - startIndex;
    return (struct parseTree){currentVariable, children, numTokens};
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

    return (struct parseTree){NULL, NULL};
}
struct parseTree getLastChild(struct parseTree parent, char *childName) {
    int i;
    for (i = parent.children->length - 1; i >= 0; i--) {
        struct parseTree child = get(struct parseTree, parent.children, i);
        if (strcmp(child.name, childName) == 0)
            return child;
    }

    return (struct parseTree){NULL, NULL};
}

