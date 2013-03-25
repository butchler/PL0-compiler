#include "generator.h"

defineVector(instructionVector, struct instruction);
defineVector(symbolVector, struct symbol);

struct instruction makeInstruction(char *instruction, int lexicalLevel, int modifier) {
    return (struct instruction){getOpcode(instruction), lexicalLevel, modifier};
}

struct parseTree getChild(struct parseTree parent, char *childName) {
    int i;
    for (i = 0; i < parent.children->length; i++) {
        struct parseTree child = parseTreeVector_get(parent.children, i);
        if (strcmp(child.name, childName) == 0)
            return child;
    }

    return (struct parseTree){NULL, NULL, NULL};
}

struct instructionVector *generateInstructions(struct parseTree tree) {
    return generate(tree, symbolVector_init());
}

struct instructionVector *generate(struct parseTree tree, struct symbolVector *symbols) {
    if (tree.name == NULL)
        return NULL;

    struct instructionVector *result = instructionVector_init();

    if (strcmp(tree.name, "program") == 0) {
        return generate(parseTreeVector_get(tree.children, 0), symbols);
    } else if (strcmp(tree.name, "block") == 0) {
        struct parseTree vars = getChild(tree, "var-declaration");
        struct parseTree statement = getChild(tree, "statement");

        if (vars.name == NULL || statement.name == NULL)
            goto fail;

        return instructionVector_concat(generate(vars, symbols), generate(statement, symbols));
    } else if (strcmp(tree.name, "var-declaration") == 0) {
        struct parseTree identifiers = getChild(tree, "identifiers");
        int numVars = 0;
        while (identifiers.name != NULL) {
            struct parseTree identifier = getChild(identifiers, "identifier");
            if (identifier.name == NULL)
                goto fail;

            char *name = stringVector_get(identifier.data, 0);
            symbolVector_push(symbols, makeSymbol(name));
            numVars += 1;

            identifiers = getChild(identifiers, "identifiers");
        }

        instructionVector_push(result, makeInstruction("inc", 0, numVars));
    } else if (strcmp(tree.name, "statement") == 0) {
        return generate(parseTreeVector_get(tree.children, 0), symbols);
    } else if (strcmp(tree.name, "begin-block") == 0) {
        struct parseTree statements = getChild(tree, "statements");

        return generate(statements, symbols);
    } else if (strcmp(tree.name, "statements") == 0) {
        struct parseTree statements = getChild(tree, "statements");
        while (statements.name != NULL) {
            struct parseTree statement = getChild(statements, "statement");
            if (statement.name == NULL)
                goto fail;

            instructionVector_concat(result, generate(statement, symbols));

            statements = getChild(statements, "statements");
        }
    } else if (strcmp(tree.name, "read-statement") == 0) {
        struct parseTree identifier = getChild(tree, "identifier");
        char *name = stringVector_get(identifier.data, 0);

        instructionVector_push(result, makeInstruction("sio", 0, 2));
        instructionVector_push(result, makeInstruction("lod",
                    getLevel(symbols, name), getAddress(symbols, name)));
    } else {
        goto fail;
    }

    return result;

fail:
    instructionVector_free(result);
    return NULL;
}

