#include "src/lib/parser.h"
#include "src/lib/util.h"
#include "src/pl0.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define STACK_FRAME_SIZE 4

// A symbol can be a variable name or a procedure name. We need to keep track
// of its lexical level so we know what code can access it, and we need to keep
// track of its address so we can load its value.
struct symbol {
    char *name;
    int type;      // Whether the symbol is of a variable, a constant, or a procedure.
    int level;     // The lexical level of the symbol.
    int address;   // The address of the symbol on the stack, in it's lexical
                   // level, or the address in the code if it's a procedure.
    int constantValue;     // If it's a constant, holds the value of the constant.
};
// Symbol types
enum { VARIABLE = 1, CONSTANT, PROCEDURE };

// Used in the generate function to keep track of the current state.
struct generatorState {
    struct vector *symbols;   // The symbol table.
    //int currentAddress;   // The current code address.
    int currentLevel;     // The current lexical level.
    struct vector *instructions;   // The instructions that have been generated so far.
    struct generatorState *parentState;
};

// Error fucntions.
void addGeneratorError(char *errorMessage);
void clearGeneratorErrors();

// Functions used by generatorInstructions.
// ========================================
void generate(struct parseTree tree, struct generatorState *state);
void generate_program(struct parseTree tree, struct generatorState *state);
void generate_block(struct parseTree tree, struct generatorState *state);
void generate_varDeclaration(struct parseTree tree, struct generatorState *state);
void generate_vars(struct parseTree tree, struct generatorState *state);
void generate_var(struct parseTree tree, struct generatorState *state);
void generate_constDeclaration(struct parseTree tree, struct generatorState *state);
void generate_constants(struct parseTree tree, struct generatorState *state);
void generate_constant(struct parseTree tree, struct generatorState *state);
void generate_procedureDeclaration(struct parseTree tree, struct generatorState *state);
void generate_procedures(struct parseTree tree, struct generatorState *state);
void generate_procedure(struct parseTree tree, struct generatorState *state);
void generate_statement(struct parseTree tree, struct generatorState *state);
void generate_statements(struct parseTree tree, struct generatorState *state);
void generate_beginBlock(struct parseTree tree, struct generatorState *state);
void generate_readStatement(struct parseTree tree, struct generatorState *state);
void generate_writeStatement(struct parseTree tree, struct generatorState *state);
void generate_ifStatement(struct parseTree tree, struct generatorState *state);
void generate_whileStatement(struct parseTree tree, struct generatorState *state);
void generate_callStatement(struct parseTree tree, struct generatorState *state);
void generate_assignment(struct parseTree tree, struct generatorState *state);
void generate_condition(struct parseTree tree, struct generatorState *state);
void generate_relationalOperator(struct parseTree tree, struct generatorState *state);
void generate_expression(struct parseTree tree, struct generatorState *state);
void generate_addOrSubtract(struct parseTree tree, struct generatorState *state);
void generate_term(struct parseTree tree, struct generatorState *state);
void generate_multiplyOrDivide(struct parseTree tree, struct generatorState *state);
void generate_factor(struct parseTree tree, struct generatorState *state);
void generate_sign(struct parseTree tree, struct generatorState *state);
void generate_number(struct parseTree tree, struct generatorState *state);
void generate_identifier(struct parseTree tree, struct generatorState *state);

// Functions for creating and modifying generatorState structs.
// ============================================================
struct generatorState *makeGeneratorState();
void addInstruction(struct generatorState *state, char *instruction, int level, int modifier);
void addLoadInstruction(struct generatorState *state, struct parseTree identifierTree);
void addStoreInstruction(struct generatorState *state, struct parseTree identifierTree);
void addVariable(struct generatorState *state, struct parseTree identifierTree);
void addConstant(struct generatorState *state, struct parseTree identifierTree,
        struct parseTree numberTree);
void addProcedure(struct generatorState *state, struct parseTree identifierTree, int address);
struct symbol getSymbol(struct generatorState *state, char *name);

// Functions used by addInstruction.
// Utility function to initialize a struct instruction.
struct instruction makeInstruction(char *instruction, int lexicalLevel, int modifier);
// Given a string represtation of an instruction, such as "lit" or "sto",
// return the corresponding integer opcode.
int getOpcode(char *instruction);

// Implementation
// ===========================================================
struct vector *generatePL0(struct parseTree tree) {
    clearGeneratorErrors();

    struct generatorState *state = makeGeneratorState();
    generate(tree, state);
    return state->instructions;
}

void printInstructions(struct vector *instructions, int humanReadable) {
    forVector(instructions, i, struct instruction, instruction,
        int lineNumber = i;
        if (humanReadable)
            printf("%3d %-5s %-3d %-3d\n", lineNumber, instruction.opcodeName,
                    instruction.lexicalLevel, instruction.modifier);
        else
            printf("%d %d %d\n", instruction.opcode,
                    instruction.lexicalLevel, instruction.modifier););
}

void generate(struct parseTree tree, struct generatorState *state) {
    // Don't generate anything if the tree is invalid.
    if (isParseTreeError(tree))
        return;

    // tree.name == NULL will cause the strcmp's to segfault.
    assert(tree.name != NULL);

    int is(char *name) {
        return (strcmp(tree.name, name) == 0);
    }
    void call(void (*generateFunction)(struct parseTree, struct generatorState*)) {
        (*generateFunction)(tree, state);
    }

    if (is("@program")) call(generate_program);
    else if (is("@block")) call(generate_block);
    else if (is("@var-declaration")) call(generate_varDeclaration);
    else if (is("@vars")) call(generate_vars);
    else if (is("@var")) call(generate_var);
    else if (is("@const-declaration")) call(generate_constDeclaration);
    else if (is("@constants")) call(generate_constants);
    else if (is("@constant")) call(generate_constant);
    else if (is("@procedure-declaration")) call(generate_procedureDeclaration);
    else if (is("@procedures")) call(generate_procedures);
    else if (is("@procedure")) call(generate_procedure);
    else if (is("@statement")) call(generate_statement);
    else if (is("@begin-block")) call(generate_beginBlock);
    else if (is("@statements")) call(generate_statements);
    else if (is("@read-statement")) call(generate_readStatement);
    else if (is("@write-statement")) call(generate_writeStatement);
    else if (is("@if-statement")) call(generate_ifStatement);
    else if (is("@while-statement")) call(generate_whileStatement);
    else if (is("@call-statement")) call(generate_callStatement);
    else if (is("@assignment")) call(generate_assignment);
    else if (is("@condition")) call(generate_condition);
    else if (is("@rel-op")) call(generate_relationalOperator);
    else if (is("@expression")) call(generate_expression);
    else if (is("@add-or-subtract")) call(generate_addOrSubtract);
    else if (is("@term")) call(generate_term);
    else if (is("@multiply-or-divide")) call(generate_multiplyOrDivide);
    else if (is("@factor")) call(generate_factor);
    else if (is("@sign")) call(generate_sign);
    else if (is("@number")) call(generate_number);
    else if (is("@identifier")) call(generate_identifier);
}

void generate_program(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@block"));

    generate(getChild(tree, "@block"), state);
    // Add a return instruction at the end of the program.
    addInstruction(state, "opr", 0, 0);
}

void generate_block(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@statement"));

    addInstruction(state, "inc", 0, STACK_FRAME_SIZE);
    generate(getChild(tree, "@const-declaration"), state);
    generate(getChild(tree, "@var-declaration"), state);
    generate(getChild(tree, "@procedure-declaration"), state);
    generate(getChild(tree, "@statement"), state);
}

void generate_varDeclaration(struct parseTree tree, struct generatorState *state) {
    if (hasChild(tree, "@vars")) {
        int instructionsBefore = state->instructions->length;
        generate(getChild(tree, "@vars"), state);
        int instructionsAfter = state->instructions->length;

        // One instructions is added for each variables.
        int numVariables = instructionsAfter - instructionsBefore;

        // Ignore the instructions that were generated for each variables.
        state->instructions->length -= numVariables;

        // Allocate space for the variables.
        addInstruction(state, "inc", 0, numVariables);
    }
}

void generate_vars(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@var"));

    generate(getChild(tree, "@var"), state);
    // If there is no "@vars" child, generate will just return without
    // adding any instructions.
    generate(getChild(tree, "@vars"), state);
}

void generate_var(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@identifier"));

    addVariable(state, getChild(tree, "@identifier"));
    // Add a fake instruction so that generate_varDeclaration can count the
    // number of variables added and add its own inc instruction.
    addInstruction(state, "inc", -1, -1);
}

void generate_constDeclaration(struct parseTree tree, struct generatorState *state) {
    generate(getChild(tree, "@constants"), state);
}

void generate_constants(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@constant"));

    generate(getChild(tree, "@constant"), state);
    // If there is no "@constants" child, generate will just return without
    // adding any instructions.
    generate(getChild(tree, "@constants"), state);
}

void generate_constant(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@identifier") && hasChild(tree, "@number"));

    addConstant(state, getChild(tree, "@identifier"), getChild(tree, "@number"));
}

void generate_procedureDeclaration(struct parseTree tree, struct generatorState *state) {
    if (hasChild(tree, "@procedures") && hasChild(getChild(tree, "@procedures"), "@procedure")) {
        addInstruction(state, "jmp", -1, -1);
        int jmpInstruction = state->instructions->length - 1;
        generate(getChild(tree, "@procedures"), state);
        int afterProcedures = state->instructions->length;

        struct instruction jmpAfterProcedures = makeInstruction("jmp", 0, afterProcedures);
        set(state->instructions, jmpInstruction, jmpAfterProcedures);
    }
}

void generate_procedures(struct parseTree tree, struct generatorState *state) {
    generate(getChild(tree, "@procedure"), state);
    generate(getChild(tree, "@procedures"), state);
}

void generate_procedure(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@identifier") && hasChild(tree, "@block"));

    addProcedure(state, getChild(tree, "@identifier"), state->instructions->length);

    struct generatorState *procedureState = makeGeneratorState();
    procedureState->currentLevel = state->currentLevel + 1;
    procedureState->parentState = state;
    procedureState->instructions = state->instructions;
    generate(getChild(tree, "@block"), procedureState);
    addInstruction(procedureState, "opr", 0, 0);

    //vector_concat(state->instructions, procedureState->instructions);
}

void generate_statement(struct parseTree tree, struct generatorState *state) {
    //assert(hasChild("@if-statement") || hasChild("@assignment") || hasChild("@while-statement") || hasChild("@begin-block") || hasChild("@read-statement") || hasChild("@write-statement"));

    generate(getFirstChild(tree), state);
}

void generate_statements(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@statement"));

    generate(getChild(tree, "@statement"), state);
    generate(getChild(tree, "@statements"), state);
}

void generate_beginBlock(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@statements"));

    generate(getChild(tree, "@statements"), state);
}

void generate_readStatement(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@identifier"));

    addInstruction(state, "read", 0, 2);
    addStoreInstruction(state, getChild(tree, "@identifier"));
}

void generate_writeStatement(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@identifier"));

    addLoadInstruction(state, getChild(tree, "@identifier"));
    addInstruction(state, "write", 0, 1);
}

void generate_assignment(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@expression") && hasChild(tree, "@identifier"));

    generate(getChild(tree, "@expression"), state);
    addStoreInstruction(state, getChild(tree, "@identifier"));
}

void generate_callStatement(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@identifier"));

    struct parseTree identifier = getChild(tree, "@identifier");
    char *procedureName = getFirstChild(identifier).name;
    struct symbol procedure = getSymbol(state, procedureName);
    int levelsBack = state->currentLevel - procedure.level;
    addInstruction(state, "cal", levelsBack, procedure.address);
}

/*void generate_ifStatement(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@condition") && hasChild(tree, "@statement"));

    generate(getChild(tree, "@condition"), state);
    // Generate a fake jpc instruction first so that we can find out what
    // instruction we need to jump to.
    addInstruction(state, "jpc", -1, -1);
    int jpcIndex = state->instructions->length - 1;
    generate(getChild(tree, "@statement"), state);
    int afterIfStatement = state->instructions->length;

    // Modify the jpc instruction to jump to the end of the if statement.
    struct instruction jpcInstruction = makeInstruction("jpc", 0, afterIfStatement);
    set(state->instructions, jpcIndex, jpcInstruction);
}*/

void generate_ifStatement(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@condition") && hasChild(tree, "@statement"));

    struct vector *statements = getChildren(tree, "@statement");
    assert(statements->length == 1 || statements->length == 2);

    if (statements->length == 1) {
        // If statement
        generate(getChild(tree, "@condition"), state);
        // Generate a fake jpc instruction first so that we can find out what
        // instruction we need to jump to.
        addInstruction(state, "jpc", -1, -1);
        int jpcIndex = state->instructions->length - 1;
        generate(getChild(tree, "@statement"), state);
        int afterIfStatement = state->instructions->length;

        // Modify the jpc instruction to jump to the end of the if statement.
        struct instruction jpcInstruction = makeInstruction("jpc", 0, afterIfStatement);
        set(state->instructions, jpcIndex, jpcInstruction);
    } else if (statements->length == 2) {
        // If-else statement
        generate(getChild(tree, "@condition"), state);
        // Generate a fake jpc instruction first so that we can find out what
        // instruction we need to jump to.
        addInstruction(state, "jpc", -1, -1);
        int jpcIndex = state->instructions->length - 1;
        generate(getChild(tree, "@statement"), state);
        addInstruction(state, "jmp", -1, -1);
        int jmpIndex = state->instructions->length - 1;
        int afterIf = state->instructions->length;
        generate(getLastChild(tree, "@statement"), state);
        int afterElse = state->instructions->length;

        // Put the correct addresses in the jump instructions.
        struct instruction jpcInstruction = makeInstruction("jpc", 0, afterIf);
        set(state->instructions, jpcIndex, jpcInstruction);
        struct instruction jmpInstruction = makeInstruction("jmp", 0, afterElse);
        set(state->instructions, jmpIndex, jmpInstruction);
    }
}

void generate_whileStatement(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@condition") && hasChild(tree, "@statement"));

    int beginning = state->instructions->length;
    generate(getChild(tree, "@condition"), state);
    // Generate a fake jpc instruction first so that we can find out what
    // instruction we need to jump to.
    addInstruction(state, "jpc", -1, -1);
    int jpcIndex = state->instructions->length - 1;
    generate(getChild(tree, "@statement"), state);
    addInstruction(state, "jmp", 0, beginning);
    int afterWhileLoop = state->instructions->length;

    // Modify the jpc instruction to jump to the end of the if statement.
    struct instruction jpcInstruction = makeInstruction("jpc", 0, afterWhileLoop);
    set(state->instructions, jpcIndex, jpcInstruction);
}

void generate_condition(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@expression")
            && (hasChild(tree, "@rel-op") || hasChild(tree, "odd")));

    if (hasChild(tree, "odd")) {
        generate(getChild(tree, "@expression"), state);
        // Add the instruction that checks for oddity.
        addInstruction(state, "opr", 0, 6);
    } else {
        generate(getChild(tree, "@expression"), state);
        generate(getLastChild(tree, "@expression"), state);
        generate(getChild(tree, "@rel-op"), state);
    }
}

void generate_expression(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@term"));

    generate(getChild(tree, "@term"), state);

    if (hasChild(tree, "@add-or-subtract")) {
        assert(hasChild(tree, "@expression"));

        generate(getChild(tree, "@expression"), state);
        generate(getChild(tree, "@add-or-subtract"), state);
    }
}

void generate_addOrSubtract(struct parseTree tree, struct generatorState *state) {
    char *plusOrMinus = getFirstChild(tree).name;

    if (strcmp(plusOrMinus, "+") == 0)
        addInstruction(state, "opr", 0, 2);
    else if (strcmp(plusOrMinus, "-") == 0)
        addInstruction(state, "opr", 0, 3);
    else
        assert(0 /* Expected + or - inside add-or-subtract. */);
}

void generate_term(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@factor"));

    generate(getChild(tree, "@factor"), state);

    if (hasChild(tree, "@multiply-or-divide")) {
        assert(hasChild(tree, "@term"));

        generate(getChild(tree, "@term"), state);
        generate(getChild(tree, "@multiply-or-divide"), state);
    }
}

void generate_multiplyOrDivide(struct parseTree tree, struct generatorState *state) {
    char *starOrSlash = getFirstChild(tree).name;

    if (strcmp(starOrSlash, "*") == 0)
        addInstruction(state, "opr", 0, 4);
    else if (strcmp(starOrSlash, "/") == 0)
        addInstruction(state, "opr", 0, 5);
    else
        assert(0 /* Expected * or / inside multiply-or-divide. */);
}

void generate_factor(struct parseTree tree, struct generatorState *state) {
    assert(hasChild(tree, "@expression") || hasChild(tree, "@identifier")
            || (hasChild(tree, "@sign") && hasChild(tree, "@number")));

    // If any of the children are not found, generate() will just return
    // without adding any instructions, so we can just try all of them without
    // checking a bunch of things (as long as we trust the parser to not do
    // something weird like pair an expression and number right next to each
    // other).
    generate(getChild(tree, "@number"), state);
    generate(getChild(tree, "@sign"), state);

    generate(getChild(tree, "@identifier"), state);

    generate(getChild(tree, "@expression"), state);
}

void generate_sign(struct parseTree tree, struct generatorState *state) {
    if (tree.children->length > 0) {
        char *sign = getFirstChild(tree).name;

        if (strcmp(sign, "-") == 0)
            addInstruction(state, "opr", 0, 1);
    }
}

void generate_number(struct parseTree tree, struct generatorState *state) {
    char *number = getFirstChild(tree).name;
    assert(isInteger(number));
    int value = atoi(number);

    addInstruction(state, "lit", 0, value);
}

void generate_identifier(struct parseTree tree, struct generatorState *state) {
    addLoadInstruction(state, tree);
}

void generate_relationalOperator(struct parseTree tree, struct generatorState *state) {
    char *operator = getFirstChild(tree).name;

    if (strcmp(operator, "=") == 0)
        addInstruction(state, "opr", 0, 8);
    else if (strcmp(operator, "<>") == 0)
        addInstruction(state, "opr", 0, 9);
    else if (strcmp(operator, "<") == 0)
        addInstruction(state, "opr", 0, 10);
    else if (strcmp(operator, "<=") == 0)
        addInstruction(state, "opr", 0, 11);
    else if (strcmp(operator, ">") == 0)
        addInstruction(state, "opr", 0, 12);
    else if (strcmp(operator, ">=") == 0)
        addInstruction(state, "opr", 0, 13);
    else
        assert(0 /* Invalid relational operator. */);
}

// Generator state functions
// =========================
struct generatorState *makeGeneratorState() {
    struct generatorState *state = malloc(sizeof (struct generatorState));

    state->symbols = makeVector(struct symbol);
    state->currentLevel = 0;
    state->instructions = makeVector(struct instruction);
    state->parentState = NULL;

    return state;
}

void addInstruction(struct generatorState *state, char *instruction, int lexicalLevel, int modifier) {
    pushLiteral(state->instructions, struct instruction,
            makeInstruction(instruction, lexicalLevel, modifier));
}

struct instruction makeInstruction(char *instruction, int lexicalLevel, int modifier) {
    return (struct instruction){getOpcode(instruction), instruction, lexicalLevel, modifier};
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
    if (strcmp(instruction, "write") == 0) return 9;
    if (strcmp(instruction, "read") == 0) return 10;

    return 0;
}

// If the given parse tree has a single child node, return the name of that
// child. Because leaf nodes represent tokens, this can be used to get the
// value of a token.
char *getToken(struct parseTree parent) {
    assert(parent.children != NULL && parent.children->length == 1);

    return getFirstChild(parent).name;
}

void addLoadInstruction(struct generatorState *state, struct parseTree identifier) {
    char *name = getToken(identifier);
    struct symbol symbol = getSymbol(state, name);

    int levelsBack = state->currentLevel - symbol.level;
    if (symbol.type == PROCEDURE)
        addGeneratorError("Cannot take value of procedure.");
    else if (symbol.type == VARIABLE)
        addInstruction(state, "lod", levelsBack, symbol.address);
    else if (symbol.type == CONSTANT)
        addInstruction(state, "lit", 0, symbol.constantValue);
}
void addStoreInstruction(struct generatorState *state, struct parseTree identifier) {
    char *name = getToken(identifier);
    struct symbol symbol = getSymbol(state, name);
    int levelsBack = state->currentLevel - symbol.level;
    if (symbol.type == PROCEDURE || symbol.type == CONSTANT)
        addGeneratorError("Cannot store into a constant or procedure.");
    else if (symbol.type == VARIABLE)
        addInstruction(state, "sto", levelsBack, symbol.address);
}

void addVariable(struct generatorState *state, struct parseTree identifierTree) {
    char *name = getToken(identifierTree);
    // TODO: Will the position in the symbol table will always correspond to
    // the correct address for the symbol in each lexical level?
    int address = state->symbols->length;
    // Account for data put on stack by CAL instruction.
    address += STACK_FRAME_SIZE;
    struct symbol symbol = {name, VARIABLE, state->currentLevel, address, 0};

    push(state->symbols, symbol);
}
void addConstant(struct generatorState *state, struct parseTree identifierTree,
        struct parseTree numberTree) {
    char *name = getToken(identifierTree);

    char *number = getToken(numberTree);
    assert(isInteger(number));
    int value = atoi(number);

    struct symbol symbol = {name, CONSTANT, state->currentLevel, 0, value};

    push(state->symbols, symbol);
}
void addProcedure(struct generatorState *state, struct parseTree identifierTree, int address) {
    char *name = getToken(identifierTree);
    struct symbol symbol = {name, PROCEDURE, state->currentLevel, address, 0};
    push(state->symbols, symbol);
}
struct symbol getSymbol(struct generatorState *state, char *name) {
    // TODO: Check for symbols in higher lexical levels.
    forVector(state->symbols, i, struct symbol, symbol,
            if (strcmp(symbol.name, name) == 0)
                return symbol;);

    if (state->parentState == NULL) {
        addGeneratorError(format("Could not find symbol '%s'.", name));

        return (struct symbol){NULL, -1, -1, -1, -1};
    } else {
        return getSymbol(state->parentState, name);
    }
}

// Error functions
// ===============
struct vector *generatorErrors = NULL;

void addGeneratorError(char *errorMessage) {
    if (generatorErrors == NULL)
        generatorErrors = makeVector(char*);

    push(generatorErrors, errorMessage);
}
void clearGeneratorErrors() {
    if (generatorErrors != NULL)
        freeVector(generatorErrors);

    generatorErrors = NULL;
}
char *getGeneratorErrors() {
    if (generatorErrors == NULL)
        return NULL;
    else
        return joinStrings(generatorErrors, "\n");
}

