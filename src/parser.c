#include "parser.h"
#include "lexer.h"
#include <string.h>

struct grammar {
    struct vector *rules;
};

struct rule {
    char *variable;
    struct vector *production;
};

int isTerminal(char *string) {
}

struct parseTree parse(struct vector *lexemes, int index, char *currentVariable, struct grammar grammar) {
    struct lexeme currentLexeme = get(struct lexeme, lexemes, index);
    char *currentToken = currentLexeme.token;

    struct vector *children = makeVector(struct parseTree);

    // Return a parseTree that indicates failure to parse.
    struct parseTree reject() {
        return (struct parseTree){NULL, NULL};
    }

    // Special cases
    if (strcmp(currentVariable, "identifier") == 0) {
        struct lexeme nextLexeme = get(struct lexeme, lexemes, index + 1);
        char *identifier = nextLexeme.token;
        pushLiteral(children, struct parseTree, {identifier, NULL});
        return (struct parseTree){"identifier", children};
    }
    if (strcmp(currentVariable, "number") == 0) {
        struct lexeme nextLexeme = get(struct lexeme, lexemes, index + 1);
        char *number = nextLexeme.token;
        pushLiteral(children, struct parseTree, {number, NULL});
        return (struct parseTree){"number", children};
    }

    int i;
    for (i = 0; i < grammar.rules->length; i++) {
        struct rule rule = get(struct rule, grammar.rules, i);
        if (strcmp(rule.variable, currentVariable) == 0) {
            int j;
            for (j = 0; j < rule.production->length; j++) {
                char *varOrTerminal = get(char*, rule.production, j);
                index += 1;
                if (isTerminal(varOrTerminal)) {
                    if (strcmp(varOrTerminal, currentToken) != 0)
                        // TODO: How to give good error messages?
                        return reject();
                } else {
                    struct parseTree child = parse(lexemes, index + 1, varOrTerminal, grammar);
                    if (child.name == NULL)
                        // TODO: How to give good error messages?
                        return reject();
                    push(children, child);
                }
            }
        }
    }

    return (struct parseTree){currentVariable, children};
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

