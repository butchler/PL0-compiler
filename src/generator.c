#include "src/generator.h"
#include "src/parser.h"
#include <string.h>
#include <stdlib.h>

struct vector *generateInstructions(struct parseTree tree) {
    struct generatorState state = (struct generatorState){makeVector(struct symbol), 0, 0};
    return generate(tree, state);
}

struct vector *generate(struct parseTree tree, struct generatorState state) {
    if (tree.name == NULL) {
        setGeneratorError("Can't generate NULL parse tree.");
        return NULL;
    }

    int is(char *name) {
        return (strcmp(tree.name, name) == 0);
    }
    struct vector *result = NULL;
    void call(struct vector *(*generateFunction)(struct parseTree, struct generatorState)) {
        result = (*generateFunction)(tree, state);
    }

    if (is("program")) call(generate_program);
    else if (is("block")) call(generate_block);
    else if (is("var-declaration")) call(generate_varDeclaration);
    else if (is("statement")) call(generate_statement);
    else if (is("begin-block")) call(generate_beginBlock);
    else if (is("statements")) call(generate_statements);
    else if (is("read-statement")) call(generate_readStatement);
    else if (is("write-statement")) call(generate_writeStatement);
    else if (is("if-statement")) call(generate_ifStatement);
    else if (is("expression")) call(generate_expression);
    else if (is("rel-op")) call(generate_relationalOperator);

    if (result == NULL)
        setGeneratorError("Unknown node type in parse tree.");

    return result;
}

struct vector *generate_program(struct parseTree tree, struct generatorState state) {
    struct vector *result = generate(get(struct parseTree, tree.children, 0), state);
    if (result == NULL)
        return NULL;

    // Always add a return instruction at the end of the program.
    struct instruction returnInstruction = makeInstruction("opr", 0, 0);
    push(result, returnInstruction);

    return result;
}

struct vector *generate_block(struct parseTree tree, struct generatorState state) {
    struct vector *result = makeVector(struct instruction);

    struct parseTree vars = getChild(tree, "var-declaration");
    if (vars.name != NULL) {
        struct vector *varDeclarationCode = generate(vars, state);
        if (varDeclarationCode == NULL)
            // Assume that the error message was already set by the call to generate.
            return NULL;

        vector_concat(result, varDeclarationCode);
        state.currentAddress += varDeclarationCode->length;
    }

    struct parseTree statement = getChild(tree, "statement");
    if (statement.name != NULL) {
        struct vector *statementCode = generate(statement, state);
        if (statementCode == NULL)
            // Assume that the error message was already set by the call to generate.
            return NULL;

        vector_concat(result, statementCode);
        state.currentAddress += statementCode->length;
    }

    return result;
}

struct vector *generate_varDeclaration(struct parseTree tree, struct generatorState state) {
    struct parseTree identifiers = getChild(tree, "identifiers");
    int numVars = 0;
    while (identifiers.name != NULL) {
        struct parseTree identifier = getChild(identifiers, "identifier");
        if (identifier.name == NULL) {
            setGeneratorError("Expected identifier inside var-declaration.");
            return NULL;
        }

        char *name = get(struct parseTree, identifier.children, 0).name;
        addSymbol(state, name);
        numVars += 1;

        identifiers = getChild(identifiers, "identifiers");
    }

    struct vector *result = makeVector(struct instruction);
    struct instruction incrementInstruction = makeInstruction("inc", 0, numVars);
    push(result, incrementInstruction);

    return result;
}

struct vector *generate_statement(struct parseTree tree, struct generatorState state) {
    return generate(get(struct parseTree, tree.children, 0), state);
}

struct vector *generate_statements(struct parseTree tree, struct generatorState state) {
    struct vector *result = makeVector(struct instruction);

    struct parseTree statements = tree;
    while (statements.name != NULL) {
        struct parseTree statement = getChild(statements, "statement");
        if (statement.name == NULL) {
            freeVector(result);
            setGeneratorError("Expected statement inside statements.");
            return NULL;
        }

        struct vector *statementCode = generate(statement, state);
        if (statementCode == NULL)
            return NULL;

        vector_concat(result, statementCode);
        state.currentAddress += statementCode->length;

        freeVector(statementCode);

        statements = getChild(statements, "statements");
    }

    return result;
}

struct vector *generate_beginBlock(struct parseTree tree, struct generatorState state) {
    return generate(getChild(tree, "statements"), state);
}

struct vector *generate_readStatement(struct parseTree tree, struct generatorState state) {
    struct parseTree identifier = getChild(tree, "identifier");
    if (identifier.name == NULL || identifier.children == NULL || identifier.children->length < 1) {
        setGeneratorError("Expected identifier inside read-statement.");
        return NULL;
    }
    char *name = get(struct parseTree, identifier.children, 0).name;

    struct vector *result = makeVector(struct instruction);

    // Read value onto stack.
    struct instruction readInstruction = makeInstruction("sio", 0, 2);
    push(result, readInstruction);
    // Store value in the given variable.
    struct symbol var = getSymbol(state, name);
    struct instruction storeInstruction = makeInstruction("sto", var.level, var.address);
    push(result, storeInstruction);

    return result;
}

struct vector *generate_writeStatement(struct parseTree tree, struct generatorState state) {
    struct parseTree identifier = getChild(tree, "identifier");
    if (identifier.name == NULL || identifier.children == NULL || identifier.children->length < 1) {
        setGeneratorError("Expected identifier inside write-statement.");
        return NULL;
    }
    char *name = get(struct parseTree, identifier.children, 0).name;

    struct vector *result = makeVector(struct instruction);

    // Push value of variable onto stack.
    struct symbol var = getSymbol(state, name);
    struct instruction loadInstruction = makeInstruction("lod", var.level, var.address);
    push(result, loadInstruction);
    // Output value at top of stack.
    struct instruction writeInstruction = makeInstruction("sio", 0, 1);
    push(result, writeInstruction);

    return result;
}

struct vector *generate_ifStatement(struct parseTree tree, struct generatorState state) {
    struct parseTree condition = getChild(tree, "condition");
    struct parseTree operator = getChild(condition, "rel-op");
    struct parseTree leftExpression = getChild(condition, "expression");
    struct parseTree rightExpression = getLastChild(condition, "expression");
    struct parseTree statement = getChild(tree, "statement");

    struct vector *leftExpressionCode = generate(leftExpression, state);
    if (leftExpressionCode == NULL)
        return NULL;
    state.currentAddress += leftExpressionCode->length;
    struct vector *rightExpressionCode = generate(rightExpression, state);
    if (rightExpressionCode == NULL)
        return NULL;
    state.currentAddress += rightExpressionCode->length;
    struct vector *operatorCode = generate(operator, state);
    if (operatorCode == NULL)
        return NULL;
    state.currentAddress += operatorCode->length;
    struct vector *statementCode = generate(statement, state);
    if (statementCode == NULL)
        return NULL;

    struct vector *result = makeVector(struct instruction);

    // The left expression code puts the result of the expression on the
    // top of the stack.
    vector_concat(result, leftExpressionCode);
    // The right expression code puts the result of the expression on the
    // top of the stack, above the result of the left expression.
    vector_concat(result, rightExpressionCode);
    // The relation operator code removes the two values at the top of the
    // stack and compares them, replacing them with a result at the top of
    // the stack.
    vector_concat(result, operatorCode);
    // jpc = jump conditional, which jumps to the given address if the
    // value at the top of the statck is a zero. If the relational operator
    // produces a zero, then that means the comparison was false, so it
    // should jump to after the statement. So, it should jump to the
    // current address plus the length of the statement, plus one to take
    // the jpc instruction into account.
    state.currentAddress += 1 + statementCode->length;
    struct instruction branchInstruction = makeInstruction("jpc", 0, state.currentAddress);
    push(result, branchInstruction);
    vector_concat(result, statementCode);

    freeVector(leftExpressionCode);
    freeVector(rightExpressionCode);
    freeVector(operatorCode);
    freeVector(statementCode);

    return result;
}

struct vector *generate_expression(struct parseTree tree, struct generatorState state) {
    struct parseTree number = getChild(tree, "number");
    if (number.name != NULL && number.children != NULL && number.children->length >= 1) {
        char *numberString = get(struct parseTree, number.children, 0).name;
        int value = atoi(numberString);

        struct vector *result = makeVector(struct instruction);
        struct instruction literalInstruction = makeInstruction("lit", 0, value);
        push(result, literalInstruction);

        return result;
    }

    struct parseTree identifier = getChild(tree, "identifier");
    if (identifier.name != NULL && identifier.children != NULL && identifier.children->length >= 1) {
        char *name = get(struct parseTree, identifier.children, 0).name;
        struct symbol symbol = getSymbol(state, name);

        struct vector *result = makeVector(struct instruction);
        struct instruction loadInstruction = makeInstruction("lod", symbol.level, symbol.address);
        push(result, loadInstruction);

        return result;
    }

    setGeneratorError("Expected number or identifier inside expression.");
    return NULL;
}

struct vector *generate_relationalOperator(struct parseTree tree, struct generatorState state) {
    if (tree.children == NULL || tree.children->length < 1) {
        setGeneratorError("Expected a relation operator (such as =, <, <=, tec.) after rel-op.");
        return NULL;
    }

    char *operator = get(struct parseTree, tree.children, 0).name;
    struct instruction instruction;

    if (strcmp(operator, "=") == 0) {
        instruction = makeInstruction("opr", 0, 8);
    }
    // TODO: The rest of the relational operators.
    else {
        setGeneratorError("Unrecognized relational operator.");
        return NULL;
    }

    struct vector *result = makeVector(struct instruction);
    push(result, instruction);

    return result;
}

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

struct symbol addSymbol(struct generatorState state, char *name) {
    struct symbol symbol = {name, state.currentLevel};
    push(state.symbols, symbol);
}
struct symbol getSymbol(struct generatorState state, char *name) {
    int i;
    for (i = 0; i < state.symbols->length; i++) {
        struct symbol symbol = get(struct symbol, state.symbols, i);
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

