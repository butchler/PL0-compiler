#include "src/parser.h"
#include "src/lexer.h"
#include "src/lib/util.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

struct parseTree parse(struct vector *lexemes, struct grammar grammar,
        char *startVariable) {
    // The auto keyword is required when declaring nested functions without
    // defining them (http://gcc.gnu.org/onlinedocs/gcc/Nested-Functions.html).
    auto struct parseTree parseVariable(int index, char *currentVariable);
    auto struct parseTree parseRule(struct rule rule, int index);
    auto int isVariable(char *string);

    // Reset parser errors.
    extern struct vector *parserErrors;
    extern int maxIndex;
    parserErrors = NULL;
    maxIndex = 0;

    return parseVariable(0, startVariable);

    // Try to parse the given variable starting at the given index in the
    // lexeme list.
    struct parseTree parseVariable(int index, char *currentVariable) {
        // For each production rule in the grammar.
        forVector(grammar.rules, i, struct rule, rule,
                // If it's a production rule for the current variable.
                if (strcmp(rule.variable, currentVariable) == 0) {
                    // Try to parse the production rule.
                    struct parseTree result = parseRule(rule, index);

                    // Return on the first production rule that succeeds.
                    if (!isParseTreeError(result))
                        return result;
                });

        // If none of the rules we found worked, or we didn't find any rules,
        // return an error.
        return errorTree();
    }

    // Try to parse the given production rule at the f
    struct parseTree parseRule(struct rule rule, int index) {
        int startIndex = index;
        struct vector *children = makeVector(struct parseTree);

        // For each variable and terminal in the production rule.
        forVector(rule.production, i, char*, varOrTerminal,
            // Special case for the the empty string, which is represented as
            // "nothing" inside of production rules. Just accept it without
            // moving to the next token.
            if (strcmp(varOrTerminal, "nothing") == 0)
                continue;

            // Return an error if we hit end of input before parsing is done.
            if (index >= lexemes->length) {
                    addParserError(
                        format("Expected '%s' but got end of input while parsing %s.",
                            varOrTerminal, rule.variable),
                        index);
                    // TODO: Free children before return.
                    return errorTree();
            }

            if (isVariable(varOrTerminal)) {
                // Try to parse the variable, and if it matches, go to the next
                // token after all of the tokens that the variable matched. If
                // it doesn't match, return an error.
                char *variableName = varOrTerminal;

                struct parseTree child = parseVariable(index, variableName);
                if (isParseTreeError(child))
                    return errorTree();

                push(children, child);
                index += child.numTokens;
            } else /* varOrTerminal is a terminal */ {
                // If the current token is the same type as the token that the
                // production rule expects, add it to the parse tree and go to
                // the next token. Otherwise, return an error.
                char *tokenType = varOrTerminal;
                struct lexeme currentLexeme = get(struct lexeme, lexemes, index);

                if (strcmp(currentLexeme.tokenType, tokenType) == 0) {
                    pushLiteral(children, struct parseTree, {currentLexeme.token, NULL, 1});
                    index += 1;
                } else {
                    addParserError(
                        format("Expected '%s' but got '%s' while parsing %s.",
                            varOrTerminal, currentLexeme.token, rule.variable),
                        index);
                    // TODO: Free children before return.
                    return errorTree();
                }
            });

        int numTokens = index - startIndex;
        return (struct parseTree){rule.variable, children, numTokens};
    }

    int isVariable(char *string) {
        forVector(grammar.rules, i, struct rule, rule,
                if (strcmp(rule.variable, string) == 0)
                    return 1;);

        return 0;
    }

    doParse();
}

struct parseTree errorTree() {
    return (struct parseTree){NULL, NULL, -1};
}

int isParseTreeError(struct parseTree tree) {
    return (tree.numTokens < 0);
}

struct parseTree getChild(struct parseTree parent, char *childName) {
    assert(parent.children != NULL);

    forVector(parent.children, i, struct parseTree, child,
        if (strcmp(child.name, childName) == 0)
            return child;);

    return errorTree();
}

struct parseTree getLastChild(struct parseTree parent, char *childName) {
    assert(parent.children != NULL);

    int i;
    for (i = parent.children->length - 1; i >= 0; i--) {
        struct parseTree child = get(struct parseTree, parent.children, i);
        if (strcmp(child.name, childName) == 0)
            return child;
    }

    return errorTree();
}

int hasChild(struct parseTree parent, char *childName) {
    struct parseTree child = getChild(parent, childName);

    return !isParseTreeError(child);
}

struct parseTree getFirstChild(struct parseTree tree) {
    if (tree.children != NULL && tree.children->length > 0)
        return get(struct parseTree, tree.children, 0);
    else
        return errorTree();
}

void freeParseTree(struct parseTree tree) {
    free(tree.name);

    if (tree.children != NULL) {
        forVector(tree.children, i, struct parseTree, child,
                freeParseTree(child););
        freeVector(tree.children);
    }
}

void addRule(struct grammar grammar, char *variable, char *productionString) {
    struct vector *production = splitString(productionString, " ");
    pushLiteral(grammar.rules, struct rule, {variable, production});
}

struct vector *parserErrors = NULL;
int maxIndex = 0;

void addParserError(char *message, int currentIndex) {
    // Only the "most successful" errors should be shown, so there is an error
    // at a higher index in the token list, get rid of all of the previous
    // errors.
    if (parserErrors == NULL || currentIndex > maxIndex) {
        maxIndex = currentIndex;
        parserErrors = makeVector(char*);
    }

    if (currentIndex == maxIndex)
        push(parserErrors, message);
}

char *getParserErrors() {
    if (parserErrors == NULL)
        return NULL;

    return joinStrings(parserErrors, "\n");
}

