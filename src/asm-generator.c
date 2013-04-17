#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "asm.h"
#include "lib/vector.h"
#include "lib/util.h"

#define PROGRAM_START_ADDRESS 0x4000

struct label {
    char *name;
    int address;
};

struct generatorState {
    int currentAddress;
    struct vector *labels;
    struct vector *instructions;
};

void findLabels(struct parseTree tree, struct generatorState *state);
struct vector *generateAssembly(struct parseTree tree, struct generatorState *state);

struct label getLabel(struct generatorState *state, char *labelName);

void addRegisterInstruction(struct generatorState *state, char *instructionName,
        int r1, int r2, int r3);
void addImmediateInstruction(struct generatorState *state, char *instructionName,
        int r1, int r2, int immediate);
void addMemoryInstruction(struct generatorState *state, char *instructionName,
        int r1, int r2, int offset);
void addBranchInstruction(struct generatorState *state, char *instructionName,
        int r1, int r2, char *labelName);
void addJumpInstruction(struct generatorState *state, char *instructionName,
        char *labelName);
void addInstruction(struct generatorState *state, uint32_t machineInstruction,
        char *humanInstruction);

// Makes sure that the given value can fit in the given number of bits
void checkSize(unsigned int value, int numBits);

int getRegister(struct parseTree tree);
int getNumber(struct parseTree tree);
char *getIdentifier(struct parseTree tree);

int getOpcode(char *instructionName);
int getFunction(char *instructionName);

char *assemble(struct parseTree tree) {
    struct generatorState state;
    state.currentAddress = PROGRAM_START_ADDRESS;
    state.labels = makeVector(struct label);

    findLabels(tree, &state);

    /*forVector(state.labels, i, struct label, label,
            printf("label: name = %s, address = %d\n", label.name, label.address););*/

    state.currentAddress = PROGRAM_START_ADDRESS;
    state.instructions = makeVector(char*);
    generateAssembly(tree, &state);

    return joinStrings(state.instructions, "\n");
}

void findLabels(struct parseTree tree, struct generatorState *state) {
    char *ruleType = tree.name;

    assert(ruleType != NULL);

    if (strcmp(ruleType, "@instruction") == 0) {
        state->currentAddress += 4;
    } else if (strcmp(ruleType, "@label") == 0) {
        char *labelName = getIdentifier(getChild(tree, "@identifier"));
        if (labelName != NULL) {
            struct label label = {labelName, state->currentAddress};
            push(state->labels, label);
        }
    }

    forVector(tree.children, i, struct parseTree, child,
            findLabels(child, state););
}

struct label getLabel(struct generatorState *state, char *labelName) {
    assert(labelName != NULL);

    forVector(state->labels, i, struct label, label,
            if (strcmp(label.name, labelName) == 0)
                return label;);

    return (struct label){NULL, 0};
}

struct vector *generateAssembly(struct parseTree tree, struct generatorState *state) {
    char *ruleType = tree.name;

    assert(ruleType != NULL);

    if (strcmp(ruleType, "@instruction") == 0) {
        char *instructionType = getFirstChild(tree).name;

        if (strcmp(instructionType, "add") == 0
                || strcmp(instructionType, "sub") == 0
                || strcmp(instructionType, "and") == 0
                || strcmp(instructionType, "or") == 0
                || strcmp(instructionType, "slt") == 0
                || strcmp(instructionType, "sltu") == 0) {
            struct vector *registers = getChildren(tree, "@register");
            int r1 = getRegister(get(struct parseTree, registers, 0));
            int r2 = getRegister(get(struct parseTree, registers, 1));
            int r3 = getRegister(get(struct parseTree, registers, 2));

            addRegisterInstruction(state, instructionType, r1, r2, r3);
        } else if (strcmp(instructionType, "addi") == 0
                || strcmp(instructionType, "lui") == 0
                || strcmp(instructionType, "slti") == 0
                || strcmp(instructionType, "sltiu") == 0) {
            struct vector *registers = getChildren(tree, "@register");
            int r1 = getRegister(get(struct parseTree, registers, 0));
            int r2;
            if (strcmp(instructionType, "lui") == 0)
                r2 = 0;
            else
                r2 = getRegister(get(struct parseTree, registers, 1));
            int immediate = getNumber(getChild(tree, "@number"));

            addImmediateInstruction(state, instructionType, r1, r2, immediate);
        } else if (strcmp(instructionType, "lw") == 0
                || strcmp(instructionType, "sw") == 0) {
            int r1 = getRegister(getChild(tree, "@register"));

            struct parseTree memAccess = getChild(tree, "@mem-access");
            int r2 = getRegister(getChild(memAccess, "@register"));
            int offset = getNumber(getChild(memAccess, "@number"));

            addMemoryInstruction(state, instructionType, r1, r2, offset);
        } else if (strcmp(instructionType, "beq") == 0) {
            struct vector *registers = getChildren(tree, "@register");
            int r1 = getRegister(get(struct parseTree, registers, 0));
            int r2 = getRegister(get(struct parseTree, registers, 1));
            char *labelName = getIdentifier(getChild(tree, "@identifier"));

            addBranchInstruction(state, instructionType, r1, r2, labelName);
        } else if (strcmp(instructionType, "j") == 0) {
            char *labelName = getIdentifier(getChild(tree, "@identifier"));

            addJumpInstruction(state, instructionType, labelName);
        } else {
            printf("Unrecognized instruction: %s\n", instructionType);
            exit(1);
        }

        state->currentAddress += 4;
    } else {
        forVector(tree.children, i, struct parseTree, child,
                generateAssembly(child, state););
    }
}

void addRegisterInstruction(struct generatorState *state, char *instructionName,
        int r1, int r2, int r3) {
    checkSize(r1, 5);
    checkSize(r2, 5);
    checkSize(r3, 5);

    // text format: instruction rd, rs, rt
    // binary format: opcode | rs | rt | rd | shift amount | function
    int opcode = getOpcode(instructionName) << 26;
    int rs = r2 << 21;
    int rt = r3 << 16;
    int rd = r1 << 11;
    int shiftAmount = 0 << 6;
    int function = getFunction(instructionName) << 0;
    uint32_t instruction = opcode | rs | rt | rd | shiftAmount | function;

    addInstruction(state, instruction, format("%s $%d, $%d, $%d", instructionName, r1, r2, r3));
}
void addImmediateInstruction(struct generatorState *state, char *instructionName,
        int r1, int r2, int immediate) {
    checkSize(r1, 5);
    checkSize(r2, 5);
    // Allow immediate to be negative for use with branch instructions.
    if (immediate < 0)
        checkSize(-immediate, 16);
    else
        checkSize(immediate, 16);

    // text format: instruction rt, rs, immediate
    // binary format: opcode | rs | rt | immediate
    int opcode = getOpcode(instructionName) << 26;
    int rs = r2 << 21;
    int rt = r1 << 16;
    // Restrict the immediate to 16 bits.
    int immediate16 = immediate & 0xffff;
    uint32_t instruction = opcode | rs | rt | immediate16;

    addInstruction(state, instruction, format("%s $%d, $%d, %d", instructionName, r1, r2, immediate));
}
void addMemoryInstruction(struct generatorState *state, char *instructionName,
        int r1, int r2, int offset) {
    addImmediateInstruction(state, instructionName, r1, r2, offset);
}
void addBranchInstruction(struct generatorState *state, char *instructionName,
        int r1, int r2, char *labelName) {
    struct label label = getLabel(state, labelName);
    assert(label.name != NULL);
    int nextInstruction = state->currentAddress + 4;
    int offset = (label.address - nextInstruction) >> 2;

    addImmediateInstruction(state, instructionName, r2, r1, offset);
}
void addJumpInstruction(struct generatorState *state, char *instructionName,
        char *labelName) {
    struct label label = getLabel(state, labelName);
    assert(label.name != NULL);

    // text format: instruction target
    // binary format: opcode | target
    int opcode = getOpcode(instructionName) << 26;
    int target = label.address >> 2;
    checkSize(target, 26);
    uint32_t instruction = opcode | target;

    addInstruction(state, instruction, format("%s %d", instructionName, label.address));
}
void addInstruction(struct generatorState *state, uint32_t machineInstruction,
        char *humanInstruction) {
    pushLiteral(state->instructions, char*, format("%08x\t # %s", machineInstruction, humanInstruction, machineInstruction));
}

void checkSize(unsigned int value, int numBits) {
    int maxValue = 2 << numBits - 1;   // maxValue = 2^(numBits) - 1
    assert(value <= maxValue);
}

int getRegister(struct parseTree tree) {
    assert(hasChild(tree, "@number") || hasChild(tree, "@identifier"));

    if (hasChild(tree, "@number")) {
        return getNumber(getChild(tree, "@number"));
    } else if (hasChild(tree, "@identifier")) {
        // Convert a named register into a number.
        char *registerName = getIdentifier(getChild(tree, "@identifier"));

        // Registers that are only a name.
        if (strcmp(registerName, "zero") == 0)
            return 0;
        else if (strcmp(registerName, "gp") == 0)
            return 28;
        else if (strcmp(registerName, "sp") == 0)
            return 29;
        else if (strcmp(registerName, "fp") == 0)
            return 30;
        else if (strcmp(registerName, "ra") == 0)
            return 31;

        // Registers that start with a letter followed by a number (e.g. $t0).
        char letter = registerName[0];
        int number = atoi(registerName + 1);

        switch (letter) {
            case 'v':
                // $v0-$v1 = 2-3
                if (number == 0 || number == 1)
                    return number + 2;
                break;
            case 'a':
                // $a0-$a3 = 4-7
                if (number >= 0 && number <= 3)
                    return number + 4;
                break;
            case 't':
                if (number >= 0 || number <= 7)
                    return number + 8;   // $t0-$t7 = 8-15
                else if (number == 8 || number == 9)
                    return number + 16;  // $t8-$t9 = 24-25
                break;
            case 's':
                // $s0-$s7 = 16-23
                if (number >= 0 || number <= 7)
                    return number + 16;
                break;
        }

        return -1;
    }
}
int getNumber(struct parseTree tree) {
    return atoi(getFirstChild(tree).name);
}
char *getIdentifier(struct parseTree tree) {
    return getFirstChild(tree).name;
}

int getOpcode(char *instructionName) {
    assert(instructionName != NULL);

    struct pair { char *name; int opcode; };
    struct pair pairs[] = {
        {"add",     0},
        {"sub",     0},
        {"addi",    0b001000},
        {"and",     0},
        {"or",      0},
        {"lw",      0b100011},
        {"sw",      0b101011},
        {"lui",     0b001111},
        {"beq",     0b000100},
        {"slt",     0},
        {"slti",    0b001010},
        {"sltu",    0},
        {"sltiu",   0b001011},
        {"j",       0b000010}
    };

    int length = sizeof (pairs) / sizeof (struct pair);
    int i;
    for (i = 0; i < length; i++) {
        if (strcmp(instructionName, pairs[i].name) == 0)
            return pairs[i].opcode;
    }

    return -1;
}

int getFunction(char *instructionName) {
    assert(instructionName != NULL);

    struct pair { char *name; int function; };
    struct pair pairs[] = {
        {"add",     0b100000},
        {"sub",     0b100010},
        {"and",     0b100100},
        {"or",      0b100101},
        {"slt",     0b101010},
        {"sltu",    0b101011}
    };

    int length = sizeof (pairs) / sizeof (struct pair);
    int i;
    for (i = 0; i < length; i++) {
        if (strcmp(instructionName, pairs[i].name) == 0)
            return pairs[i].function;
    }

    return -1;
}

