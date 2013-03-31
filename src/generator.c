#include "src/generator.h"
#include "src/parser.h"
#include "src/lib/util.h"
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

    // If the node type is known and there is an error, then this error message
    // will be overwritten.
    setGeneratorError(format("Unknown node type '%s' in parse tree.", tree.name));

    if (is("program")) call(generate_program);
    else if (is("block")) call(generate_block);
    else if (is("var-declaration")) call(generate_varDeclaration);
    else if (is("statement")) call(generate_statement);
    else if (is("begin-block")) call(generate_beginBlock);
    else if (is("statements")) call(generate_statements);
    else if (is("read-statement")) call(generate_readStatement);
    else if (is("write-statement")) call(generate_writeStatement);
    else if (is("if-statement")) call(generate_ifStatement);
    else if (is("rel-op")) call(generate_relationalOperator);
    else if (is("expression")) call(generate_expression);
    else if (is("add-or-subtract")) call(generate_addOrSubtract);
    else if (is("term")) call(generate_term);
    else if (is("multiply-or-divide")) call(generate_multiplyOrDivide);
    else if (is("factor")) call(generate_factor);
    else if (is("sign")) call(generate_sign);
    else if (is("number")) call(generate_number);
    else if (is("identifier")) call(generate_identifier);

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
        addVariable(state, name, numVars);
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

    // Descent the tree of statements.
    struct parseTree statements = tree;
    while (statements.name != NULL) {
        // Get the next statement.
        struct parseTree statement = getChild(statements, "statement");
        // If there is no statement here, assume there are no more statements
        // left and exit the loop.
        if (statement.name == NULL)
            break;

        // Generate the code for the new statement and append it to the result.
        struct vector *statementCode = generate(statement, state);
        if (statementCode == NULL)
            return NULL;

        vector_concat(result, statementCode);
        state.currentAddress += statementCode->length;

        freeVector(statementCode);

        // Get the next statements node.
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
    struct parseTree term = getChild(tree, "term");
    struct parseTree addOrSubtract = getChild(tree, "add-or-subtract");
    struct parseTree expression = getChild(tree, "expression");

    // Calculate the term.
    struct vector *result = makeVector(struct instruction);
    struct vector *termCode = generate(term, state);
    if (termCode == NULL)
        return NULL;
    state.currentAddress += termCode->length;
    vector_concat(result, termCode);

    // If we are adding or subtracting to/from the term.
    if (addOrSubtract.name != NULL) {
        if (expression.name == NULL) {
            setGeneratorError("Expected expression after add-or-subtract.");
            return NULL;
        }

        // Calculate the expression.
        struct vector *expressionCode = generate(expression, state);
        if (expressionCode == NULL)
            return NULL;
        state.currentAddress += expressionCode->length;
        vector_concat(result, expressionCode);

        // Afterward, the results of calculating the term and expression should
        // be at the top of the stack, so we just need to add an add or
        // subtract instruction at the very end and it will take them off the
        // stack, add/subtract them, and push the result to the stack.
        struct vector *addOrSubtractCode = generate(addOrSubtract, state);
        if (addOrSubtractCode == NULL)
            return NULL;
        vector_concat(result, addOrSubtractCode);
    }

    return result;
}

struct vector *generate_addOrSubtract(struct parseTree tree, struct generatorState state) {
    struct vector *result = makeVector(struct instruction);

    char *plusOrMinus = get(char*, tree.children, 0);
    if (strcmp(plusOrMinus, "+") == 0) {
        // + = opr 0 2
        pushLiteral(result, struct instruction, makeInstruction("opr", 0, 2));
    } else if (strcmp(plusOrMinus, "-") == 0) {
        // - = opr 0 3
        pushLiteral(result, struct instruction, makeInstruction("opr", 0, 3));
    } else {
        setGeneratorError("Expected + or - inside add-or-substract");
        return NULL;
    }

    return result;
}

struct vector *generate_term(struct parseTree tree, struct generatorState state) {
    struct parseTree factor = getChild(tree, "factor");
    struct parseTree multiplyOrDivide = getChild(tree, "multiply-or-divide");
    struct parseTree term = getChild(tree, "term");

    // Calculate the factor.
    struct vector *result = makeVector(struct instruction);
    struct vector *factorCode = generate(factor, state);
    if (factorCode == NULL)
        return NULL;
    state.currentAddress += factorCode->length;
    vector_concat(result, factorCode);

    // If we are adding or subtracting to/from the term.
    if (multiplyOrDivide.name != NULL) {
        if (term.name == NULL) {
            setGeneratorError("Expected term after multiply-or-divide.");
            return NULL;
        }

        // Calculate the term.
        struct vector *termCode = generate(term, state);
        if (termCode == NULL)
            return NULL;
        state.currentAddress += termCode->length;
        vector_concat(result, termCode);

        // Multipy or divide the results of the factor and term.
        struct vector *multiplyOrDivideCode = generate(multiplyOrDivide, state);
        if (multiplyOrDivideCode == NULL)
            return NULL;
        vector_concat(result, multiplyOrDivideCode);
    }

    return result;
}

struct vector *generate_multiplyOrDivide(struct parseTree tree, struct generatorState state) {
    struct vector *result = makeVector(struct instruction);

    char *starOrSlash = get(char*, tree.children, 0);
    if (strcmp(starOrSlash, "*") == 0) {
        // * = opr 0 4
        pushLiteral(result, struct instruction, makeInstruction("opr", 0, 4));
    } else if (strcmp(starOrSlash, "/") == 0) {
        // / = opr 0 5
        pushLiteral(result, struct instruction, makeInstruction("opr", 0, 5));
    } else {
        setGeneratorError("Expected * or / inside multiply-or-divide");
        return NULL;
    }

    return result;
}

struct generatorState concatCodeFor(struct parseTree tree, struct vector *destination,
        struct generatorState state) {
    struct vector *code = generate(tree, state);
    if (code == NULL)
        return state;
    vector_concat(destination, code);
    state.currentAddress += code->length;
    return state;
}

struct vector *generate_factor(struct parseTree tree, struct generatorState state) {
    struct parseTree expression = getChild(tree, "expression");
    struct parseTree sign = getChild(tree, "sign");
    struct parseTree number = getChild(tree, "number");
    struct parseTree identifier = getChild(tree, "identifier");

    struct vector *result = makeVector(struct instruction);

    if (expression.name != NULL) {
        concatCodeFor(expression, result, state);
        return result;
    }

    if (identifier.name != NULL) {
        concatCodeFor(identifier, result, state);
        return result;
    }

    if (number.name != NULL && sign.name != NULL) {
        // Calculate the number.
        state = concatCodeFor(number, result, state);
        // Possibly negate the number.
        state = concatCodeFor(sign, result, state);

        return result;
    }

    setGeneratorError("Expected expression, identifier or sign and number inside 'factor'.");
    return NULL;
}

struct vector *generate_sign(struct parseTree tree, struct generatorState state) {
    struct vector *result = makeVector(struct instruction);

    // If there is a sign and the sign is -, add a negation instruction.
    if (tree.children->length > 0) {
        char *sign = get(char*, tree.children, 0);
        if (strcmp(sign, "-") == 0) {
            // Negate = opr 0 1
            pushLiteral(result, struct instruction, makeInstruction("opr", 0, 1));
        }
    }

    return result;
}

struct vector *generate_number(struct parseTree tree, struct generatorState state) {
    struct vector *result = makeVector(struct instruction);

    if (tree.children->length < 1) {
        setGeneratorError("Expected integer value inside 'number'.");
        return NULL;
    }

    // Output a literal instruction that pushes the value of the number to the stack.
    char *number = get(char*, tree.children, 0);
    if (isInteger(number)) {
        int value = atoi(number);
        pushLiteral(result, struct instruction, makeInstruction("lit", 0, value));
        return result;
    } else {
        setGeneratorError("Invalid integer value inside 'number'.");
        return NULL;
    }
}

struct vector *generate_identifier(struct parseTree tree, struct generatorState state) {
    if (tree.children->length < 1) {
        setGeneratorError("Expected identifier name inside 'identifier'.");
        return NULL;
    }

    struct vector *result = makeVector(struct instruction);

    char *name = get(char*, tree.children, 0);
    struct symbol var = getSymbol(state, name);
    if (var.type == VARIABLE) {
        pushLiteral(result, struct instruction, makeInstruction("lod", var.level, var.address));
    } else if (var.type == CONSTANT) {
        pushLiteral(result, struct instruction, makeInstruction("lit", 0, var.constantValue));
    }

    return result;
}

/*struct vector *generate_expression(struct parseTree tree, struct generatorState state) {
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
}*/

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

struct symbol addVariable(struct generatorState state, char *name, int address) {
    struct symbol symbol = {name, VARIABLE, state.currentLevel, address, 0};
    push(state.symbols, symbol);
    return symbol;
}
struct symbol addConstant(struct generatorState state, char *name, int value) {
    struct symbol symbol = {name, CONSTANT, state.currentLevel, 0, value};
    push(state.symbols, symbol);
    return symbol;
}
struct symbol getSymbol(struct generatorState state, char *name) {
    // TODO: Check for symbols in higher lexical levels.
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

