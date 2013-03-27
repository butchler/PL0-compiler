#include "generator.h"
#include <assert.h>

struct symbol {
    char *name;
    int level;     // The lexical level of the symbol.
    int address;   // The address of the symbol on the stack, in it's lexical level.
};

declareVector(symbolVector, struct symbol)
defineVector(symbolVector, struct symbol)

// Used in the generate function to keep track of the current state.
struct generatorState {
    struct symbolVector *symbols;
    int currentAddress;   // The current code address.
    int currentLevel;     // The current lexical level.
};

defineVector(instructionVector, struct instruction)

int getOpcode(char *instruction) {
    if (strcmp(instruction, "lit") == 0) return 1;
    if (strcmp(instruction, "opr") == 0) return 2;
    if (strcmp(instruction, "lod") == 0) return 3;
    if (strcmp(instruction, "sto") == 0) return 4;
    if (strcmp(instruction, "cal") == 0) return 5;
    if (strcmp(instruction, "inc") == 0) return 6;
    if (strcmp(instruction, "jmp") == 0) return 7;
    if (strcmp(instruction, "jpc") == 0) return 8;
    if (strcmp(instruction, "sio") == 0) return 9;

    return 0;
}

struct instruction makeInstruction(char *instruction, int lexicalLevel, int modifier) {
    return (struct instruction){getOpcode(instruction), instruction, lexicalLevel, modifier};
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
struct parseTree getLastChild(struct parseTree parent, char *childName) {
    int i;
    for (i = parent.children->length - 1; i >= 0; i--) {
        struct parseTree child = parseTreeVector_get(parent.children, i);
        if (strcmp(child.name, childName) == 0)
            return child;
    }

    return (struct parseTree){NULL, NULL, NULL};
}

struct symbol addSymbol(struct generatorState state, char *name) {
    symbolVector_push(state.symbols, (struct symbol){name, state.currentLevel});
}
struct symbol getSymbol(struct generatorState state, char *name) {
    int i;
    for (i = 0; i < state.symbols->length; i++) {
        struct symbol symbol = symbolVector_get(state.symbols, i);
        if (strcmp(symbol.name, name) == 0)
            return symbol;
    }

    return (struct symbol){NULL, 0, 0};
}

char *generator_error = "";

void setGeneratorError(char *errorMessage) {
    generator_error = errorMessage;
}
char *getGeneratorError() {
    return generator_error;
}

struct instructionVector *generate(struct parseTree tree, struct generatorState state) {
    if (tree.name == NULL) {
        setGeneratorError("Can't generate NULL parse tree.");
        return NULL;
    }

    if (strcmp(tree.name, "program") == 0) {
        struct instructionVector *result = generate(parseTreeVector_get(tree.children, 0), state);

        // Always add a return instruction at the end of the program.
        instructionVector_push(result, makeInstruction("opr", 0, 0));

        return result;
    } else if (strcmp(tree.name, "block") == 0) {
        struct instructionVector *result = instructionVector_init();

        struct parseTree vars = getChild(tree, "var-declaration");
        if (vars.name != NULL) {
            struct instructionVector *varDeclarationCode = generate(vars, state);
            if (varDeclarationCode == NULL)
                // Assume that the error message was already set by the call to generate.
                return NULL;

            instructionVector_concat(result, varDeclarationCode);
            state.currentAddress += varDeclarationCode->length;
        }

        struct parseTree statement = getChild(tree, "statement");
        if (statement.name != NULL) {
            struct instructionVector *statementCode = generate(statement, state);
            if (statementCode == NULL)
                // Assume that the error message was already set by the call to generate.
                return NULL;

            instructionVector_concat(result, statementCode);
            state.currentAddress += statementCode->length;
        }

        return result;
    } else if (strcmp(tree.name, "var-declaration") == 0) {
        struct parseTree identifiers = getChild(tree, "identifiers");
        int numVars = 0;
        while (identifiers.name != NULL) {
            struct parseTree identifier = getChild(identifiers, "identifier");
            if (identifier.name == NULL) {
                setGeneratorError("Expected identifier inside var-declaration.");
                return NULL;
            }

            char *name = stringVector_get(identifier.data, 0);
            addSymbol(state, name);
            numVars += 1;

            identifiers = getChild(identifiers, "identifiers");
        }

        struct instructionVector *result = instructionVector_init();
        instructionVector_push(result, makeInstruction("inc", 0, numVars));

        return result;
    } else if (strcmp(tree.name, "statement") == 0) {
        return generate(parseTreeVector_get(tree.children, 0), state);
    } else if (strcmp(tree.name, "begin-block") == 0) {
        return generate(getChild(tree, "statements"), state);
    } else if (strcmp(tree.name, "statements") == 0) {
        struct instructionVector *result = instructionVector_init();

        struct parseTree statements = tree;
        while (statements.name != NULL) {
            struct parseTree statement = getChild(statements, "statement");
            if (statement.name == NULL) {
                instructionVector_free(result);
                setGeneratorError("Expected statement inside statements.");
                return NULL;
            }

            struct instructionVector *statementCode = generate(statement, state);
            if (statementCode == NULL)
                return NULL;

            instructionVector_concat(result, statementCode);
            state.currentAddress += statementCode->length;

            instructionVector_free(statementCode);

            statements = getChild(statements, "statements");
        }

        return result;
    } else if (strcmp(tree.name, "read-statement") == 0) {
        struct parseTree identifier = getChild(tree, "identifier");
        if (identifier.name == NULL || identifier.data == NULL || identifier.data->length < 1) {
            setGeneratorError("Expected identifier inside read-statement.");
            return NULL;
        }
        char *name = stringVector_get(identifier.data, 0);

        struct instructionVector *result = instructionVector_init();

        // Read value onto stack.
        instructionVector_push(result, makeInstruction("sio", 0, 2));
        // Store value in the given variable.
        struct symbol var = getSymbol(state, name);
        instructionVector_push(result, makeInstruction("sto", var.level, var.address));

        return result;
    } else if (strcmp(tree.name, "write-statement") == 0) {
        struct parseTree identifier = getChild(tree, "identifier");
        if (identifier.name == NULL || identifier.data == NULL || identifier.data->length < 1) {
            setGeneratorError("Expected identifier inside write-statement.");
            return NULL;
        }
        char *name = stringVector_get(identifier.data, 0);

        struct instructionVector *result = instructionVector_init();

        // Push value of variable onto stack.
        struct symbol var = getSymbol(state, name);
        instructionVector_push(result, makeInstruction("lod", var.level, var.address));
        // Output value at top of stack.
        instructionVector_push(result, makeInstruction("sio", 0, 1));

        return result;
    } else if (strcmp(tree.name, "if-statement") == 0) {
        struct parseTree condition = getChild(tree, "condition");
        struct parseTree operator = getChild(condition, "rel-op");
        struct parseTree leftExpression = getChild(condition, "expression");
        struct parseTree rightExpression = getLastChild(condition, "expression");
        struct parseTree statement = getChild(tree, "statement");

        struct instructionVector *leftExpressionCode = generate(leftExpression, state);
        if (leftExpressionCode == NULL)
            return NULL;
        state.currentAddress += leftExpressionCode->length;
        struct instructionVector *rightExpressionCode = generate(rightExpression, state);
        if (rightExpressionCode == NULL)
            return NULL;
        state.currentAddress += rightExpressionCode->length;
        struct instructionVector *operatorCode = generate(operator, state);
        if (operatorCode == NULL)
            return NULL;
        state.currentAddress += operatorCode->length;
        struct instructionVector *statementCode = generate(statement, state);
        if (statementCode == NULL)
            return NULL;

        struct instructionVector *result = instructionVector_init();

        // The left expression code puts the result of the expression on the
        // top of the stack.
        instructionVector_concat(result, leftExpressionCode);
        // The right expression code puts the result of the expression on the
        // top of the stack, above the result of the left expression.
        instructionVector_concat(result, rightExpressionCode);
        // The relation operator code removes the two values at the top of the
        // stack and compares them, replacing them with a result at the top of
        // the stack.
        instructionVector_concat(result, operatorCode);
        // jpc = jump conditional, which jumps to the given address if the
        // value at the top of the statck is a zero. If the relational operator
        // produces a zero, then that means the comparison was false, so it
        // should jump to after the statement. So, it should jump to the
        // current address plus the length of the statement, plus one to take
        // the jpc instruction into account.
        state.currentAddress += 1 + statementCode->length;
        instructionVector_push(result, makeInstruction("jpc", 0, state.currentAddress));
        instructionVector_concat(result, statementCode);

        instructionVector_free(leftExpressionCode);
        instructionVector_free(rightExpressionCode);
        instructionVector_free(operatorCode);
        instructionVector_free(statementCode);

        return result;
    } else if (strcmp(tree.name, "expression") == 0) {
        struct parseTree number = getChild(tree, "number");
        if (number.name != NULL && number.data != NULL && number.data->length >= 1) {
            char *numberString = stringVector_get(number.data, 0);
            int value = atoi(numberString);

            struct instructionVector *result = instructionVector_init();
            instructionVector_push(result, makeInstruction("lit", 0, value));

            return result;
        }

        struct parseTree identifier = getChild(tree, "identifier");
        if (identifier.name != NULL && identifier.data != NULL && identifier.data->length >= 1) {
            char *name = stringVector_get(identifier.data, 0);
            struct symbol symbol = getSymbol(state, name);

            struct instructionVector *result = instructionVector_init();
            instructionVector_push(result, makeInstruction("lod", symbol.level, symbol.address));

            return result;
        }

        setGeneratorError("Expected number or identifier inside expression.");
        return NULL;
    } else if (strcmp(tree.name, "rel-op") == 0) {
        if (tree.data == NULL || tree.data->length < 1) {
            setGeneratorError("Expected a relation operator (such as =, <, <=, tec.) after rel-op.");
            return NULL;
        }

        char *operator = stringVector_get(tree.data, 0);
        struct instruction instruction;

        if (strcmp(operator, "=") == 0) {
            instruction = makeInstruction("opr", 0, 8);
        }
        // TODO: The rest of the relational operators.
        else {
            setGeneratorError("Unrecognized relational operator.");
            return NULL;
        }

        struct instructionVector *result = instructionVector_init();
        instructionVector_push(result, instruction);

        return result;
    }

    setGeneratorError("Unknown node type in parse tree.");
    return NULL;
}

struct instructionVector *generateInstructions(struct parseTree tree) {
    struct generatorState state = (struct generatorState){symbolVector_init(), 0, 0};
    return generate(tree, state);
}

