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
        //setGeneratorError("Can't generate NULL parse tree.");
        setGeneratorError(format("Can't generate NULL parse tree. (%d)", state.currentAddress));
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
    else if (is("vars")) call(generate_vars);
    else if (is("var")) call(generate_var);
    else if (is("statement")) call(generate_statement);
    else if (is("begin-block")) call(generate_beginBlock);
    else if (is("statements")) call(generate_statements);
    else if (is("read-statement")) call(generate_readStatement);
    else if (is("write-statement")) call(generate_writeStatement);
    else if (is("if-statement")) call(generate_ifStatement);
    else if (is("condition")) call(generate_condition);
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

// Sets an error message if the given tree doesn't have a child with the given
// name. Returns the result of getChild(tree, name).
struct parseTree expectChild(struct parseTree tree, char *name) {
    struct parseTree child = getChild(tree, name);

    if (isParseTreeError(child)) {
        setGeneratorError(format("Expected '%s' inside of '%s'.", name, tree.name));
    }

    return child;
}
struct parseTree expectLastChild(struct parseTree tree, char *name) {
    struct parseTree child = getLastChild(tree, name);

    if (isParseTreeError(child)) {
        setGeneratorError(format("Expected '%s' inside of '%s'.", name, tree.name));
    }

    return child;
}

// Takes a space-separated list of children names for the given tree and tries
// to call generate on each of the children with those names, returning the
// generated code concatenated together in the order they appear in the
// childrenNames.
struct vector *concatCode(struct parseTree tree, struct generatorState state, char *childrenNames) {
    struct vector *result = makeVector(struct instruction);
    struct vector *names = splitString(childrenNames, " ");

    int i;
    for (i = 0; i < names->length; i++) {
        char *name = get(char*, names, i);

        // Get child with the given name.
        struct parseTree child = expectChild(tree, name);
        //if (isParseTreeError(child))
        if (isParseTreeError(child) || child.name == NULL)
            return NULL;

        // Try to generate code for the child.
        struct vector *code = generate(child, state);
        if (code == NULL)
            return NULL;
        state.currentAddress += code->length;
        vector_concat(result, code);
        freeVector(code);

        free(name);
    }

    freeVector(names);

    /*if (result->length == 0)
        return NULL;*/

    return result;
}

// Takes a tree that represents an identifier (i.e. tree.name = "identifier"),
// and returns the name of the identifier inside the tree.
// So, given a tree that looks like (identifier x), returns "x".
char *getIdentifierName(struct parseTree identifierTree) {
    if (identifierTree.children->length < 1) {
        setGeneratorError("Expected identifier name inside 'identifier'.");
        return NULL;
    }
    struct parseTree firstChild = get(struct parseTree, identifierTree.children, 0);
    return firstChild.name;
}

struct vector *generate_program(struct parseTree tree, struct generatorState state) {
    struct vector *result = concatCode(tree, state, "block");
    if (result == NULL)
        return NULL;

    // Always add a return instruction at the end of the program.
    pushLiteral(result, struct instruction, makeInstruction("opr", 0, 0));

    return result;
}

struct vector *generate_block(struct parseTree tree, struct generatorState state) {
    return concatCode(tree, state, "var-declaration statement");
}

struct vector *generate_varDeclaration(struct parseTree tree, struct generatorState state) {
    struct vector *result = concatCode(tree, state, "vars");
    if (result == NULL)
        return NULL;
    int numVars = result->length;

    // Add an instruction to reserve space for the variables.
    // TODO: Also reserve space for the data that the CAL instruction creates.
    result = makeVector(struct instruction);
    pushLiteral(result, struct instruction, makeInstruction("inc", 0, numVars));

    return result;
}

struct vector *generate_vars(struct parseTree tree, struct generatorState state) {
    struct vector *result = concatCode(tree, state, "var vars");
    if (result != NULL)
        return result;
    else
        return concatCode(tree, state, "var");
}

struct vector *generate_var(struct parseTree tree, struct generatorState state) {
    struct parseTree identifier = expectChild(tree, "identifier");
    if (isParseTreeError(identifier))
        return NULL;

    char *identifierName = getIdentifierName(identifier);
    if (identifierName == NULL)
        return NULL;

    addVariable(state, identifierName);

    // Return a dummy instruction so that varDeclaration knows how many
    // variables were allocated.
    struct vector *result = makeVector(struct instruction);
    pushLiteral(result, struct instruction, makeInstruction("lit", -1, -1));
    return result;
}

struct vector *generate_statement(struct parseTree tree, struct generatorState state) {
    return generate(get(struct parseTree, tree.children, 0), state);
}

struct vector *generate_statements(struct parseTree tree, struct generatorState state) {
    struct vector *result = concatCode(tree, state, "statement statements");
    if (result != NULL)
        return result;
    else
        return concatCode(tree, state, "statement");
}

struct vector *generate_beginBlock(struct parseTree tree, struct generatorState state) {
    return concatCode(tree, state, "statements");
}

struct vector *generate_readStatement(struct parseTree tree, struct generatorState state) {
    struct parseTree identifier = expectChild(tree, "identifier");
    if (isParseTreeError(identifier))
        return NULL;

    char *identifierName = getIdentifierName(identifier);
    if (identifierName == NULL)
        return NULL;

    struct symbol var = getSymbol(state, identifierName);
    if (isSymbolError(var))
        return NULL;
    if (var.type != VARIABLE) {
        setGeneratorError("Cannot read into a constant or procedure.");
        return NULL;
    }

    struct vector *result = makeVector(struct instruction);
    pushLiteral(result, struct instruction, makeInstruction("sio", 0, 2));
    pushLiteral(result, struct instruction, makeInstruction("sto", var.level, var.address));
    return result;
}

struct vector *generate_writeStatement(struct parseTree tree, struct generatorState state) {
    // Get the code that loads the identifier onto the stack.
    struct vector *result = concatCode(tree, state, "identifier");
    // Add instruction to output the loaded value.
    if (result == NULL)
        return NULL;
    pushLiteral(result, struct instruction, makeInstruction("sio", 0, 1));
    return result;
}

struct vector *generate_ifStatement(struct parseTree tree, struct generatorState state) {
    // Generate the code that will put the result of the condition onto the stack.
    struct vector *conditionCode = concatCode(tree, state, "condition");
    if (conditionCode == NULL)
        return NULL;
    // Add 1 to simulate the jpc instruction that follows the condition code.
    state.currentAddress += conditionCode->length + 1;

    // Generate the statement code first so we can find out how long it is.
    struct vector *statementCode = concatCode(tree, state, "statement");
    if (statementCode == NULL)
        return NULL;

    // Add a jpc instruction to jump to after the if statement if the condition failed.
    struct vector *result = makeVector(struct instruction);
    vector_concat(result, conditionCode);
    pushLiteral(result, struct instruction,
            makeInstruction("jpc", 0, state.currentAddress + statementCode->length));
    vector_concat(result, statementCode);

    return result;
}

struct vector *generate_condition(struct parseTree tree, struct generatorState state) {
    struct parseTree oddsym = expectChild(tree, "odd");
    if (!isParseTreeError(oddsym)) {
        struct vector *result = concatCode(tree, state, "expression");
        if (result == NULL)
            return NULL;
        // opr 0 6 = odd, replaces the top element of the stack with whether or
        // not it's value was odd.
        pushLiteral(result, struct instruction, makeInstruction("opr", 0, 6));
    } else {
        struct parseTree leftExpression = expectChild(tree, "expression");
        struct parseTree rightExpression = expectLastChild(tree, "expression");
        if (isParseTreeError(leftExpression) || isParseTreeError(rightExpression))
            return NULL;

        // Generate left expression code.
        struct vector *leftExpressionCode = generate(leftExpression, state);
        if (leftExpressionCode == NULL)
            return NULL;
        state.currentAddress += leftExpressionCode->length;

        // Generate right expression code.
        struct vector *rightExpressionCode = generate(rightExpression, state);
        if (rightExpressionCode == NULL)
            return NULL;
        state.currentAddress += rightExpressionCode->length;

        // Generate relational operator code.
        struct vector *operatorCode = concatCode(tree, state, "rel-op");
        if (operatorCode == NULL)
            return NULL;

        // Put it all together.
        struct vector *result = makeVector(struct instruction);
        vector_concat(result, leftExpressionCode);
        vector_concat(result, rightExpressionCode);
        vector_concat(result, operatorCode);

        return result;
    }
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

struct symbol addVariable(struct generatorState state, char *name) {
    // Use it's position in the symbol table as the address.
    int address = state.symbols->length;
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
// Returns true if the given symbol represents an error in getSymbol.
int isSymbolError(struct symbol symbol) {
    return (symbol.name == NULL);
}

char *generator_error = "";

void setGeneratorError(char *errorMessage) {
    generator_error = errorMessage;
}
char *getGeneratorError() {
    return generator_error;
}

