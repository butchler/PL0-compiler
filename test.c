#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "generator.h"
#include "parser.h"
#include "vector.h"
#include "vector-types.h"

defineVector(parseTreeVector, struct parseTree)   // TODO: Move to parser.c when it exists.
defineVector(stringVector, char*)
defineVector(instructionVector, struct instruction)

// Turns the lisp-like form (a (b c) d) into a vector of strings containing
// "a", "(b c)", and "d".
struct stringVector *vectorizeForm(char *form) {
    char *formStart = strchr(form, '(');
    char *formEnd = strrchr(form, ')');

    char *skipWhitespace(char *start) {
        int numWhitespace = strspn(start, " \r\n\t");
        return start + numWhitespace;
    }
    char *skipNonWhitespace(char *start) {
        int numNonWhitespace = strcspn(start, " \r\n\t");
        return start + numNonWhitespace;
    }
    char *skipForm(char *start) {
        char *c = start + 1;
        int level = 1;
        while (level > 0) {
            if (*c == '(')
                level += 1;
            else if (*c == ')')
                level -= 1;
            c += 1;
        }

        return c;
    }

    struct stringVector *items = stringVector_init();
    char *c = formStart + 1;
    while (c < formEnd) {

        char *start = skipWhitespace(c);

        char *end = NULL;
        if (*start == '(')
            end = skipForm(start);
        else
            end = skipNonWhitespace(start);
        if (end > formEnd)
            end = formEnd;

        int length = end - start;
        char *item = strndup(start, length);
        stringVector_push(items, item);

        c = end;
    }

    return items;
}

// Awesome alias for generateParseTree so that you don't have to type a bunch
// of quotes or \'s to make a multiline string.
#define pt(forms) generateParseTree("(" #forms ")")

// Generates a parseTree struct from a string of lisp-like forms that
// represent the structure of the parse tree.
struct parseTree generateParseTree(char *forms) {
    struct stringVector *items = vectorizeForm(forms);

    char *name = stringVector_get(items, 0);

    struct parseTreeVector *children = parseTreeVector_init();
    struct stringVector *data = stringVector_init();
    int i;
    for (i = 1; i < items->length; i++) {
        char *item = stringVector_get(items, i);
        if (item[0] == '(') {
            struct parseTree childTree = generateParseTree(item);
            parseTreeVector_push(children, childTree);
        } else {
            stringVector_push(data, item);
        }
    }

    return (struct parseTree){name, children, data};
}

int instructionsEqual(struct instructionVector *instructions, char *expectedInstructions) {
    int i; int offset = 0;
    for (i = 0; i < instructions->length; i++) {
        struct instruction currentInstruction = instructionVector_get(instructions, i);

        char instruction[3]; int lexicalLevel; int modifier; int bytesRead;
        int numMatched = sscanf(expectedInstructions + offset, "%s %d %d%n", instruction, &lexicalLevel, &modifier, &bytesRead);
        offset += bytesRead;

        if (numMatched < 3)
            return 0;

        int opcode = getOpcode(instruction);

        if (opcode != currentInstruction.opcode ||
                lexicalLevel != currentInstruction.lexicalLevel ||
                modifier != currentInstruction.modifier)
            return 0;
    }

    if (offset == strlen(expectedInstructions))
        return 1;

    return 0;
}

// Test the code that was defined in this file to make testing easier.
void testTest() {
    void testVectorizeForm() {
        struct stringVector *items = vectorizeForm("(aaa (bbb ccc) ddd)");

        assert(strcmp(stringVector_get(items, 0), "aaa") == 0);
        assert(strcmp(stringVector_get(items, 1), "(bbb ccc)") == 0);
        assert(strcmp(stringVector_get(items, 2), "ddd") == 0);

        stringVector_free(items);
    }

    void testGenerateParseTree() {
        struct parseTree tree = pt(aaa (bbb (identifier ccc)) (number 123));
        assert(strcmp(tree.name, "aaa") == 0);
        struct parseTree child = parseTreeVector_get(tree.children, 0);
        assert(strcmp(child.name, "bbb") == 0);
        struct parseTree grandchild = parseTreeVector_get(child.children, 0);
        assert(strcmp(grandchild.name, "identifier") == 0);
        char *identifier = stringVector_get(grandchild.data, 0);
        assert(strcmp(identifier, "ccc") == 0);
        struct parseTree child2 = parseTreeVector_get(tree.children, 1);
        assert(strcmp(child2.name, "number") == 0);

        // TODO: Free parse tree.
    }

    void testInstructionsEqual() {
        struct instructionVector *instructions = instructionVector_init();
        instructionVector_push(instructions, (struct instruction){1, 0, 0});
        instructionVector_push(instructions, (struct instruction){2, 0, 0});
        instructionVector_push(instructions, (struct instruction){3, 0, 0});
        assert(instructionsEqual(instructions,
                    " lit 0 0"
                    " opr 0 0"
                    " lod 0 0"));
        assert(!instructionsEqual(instructions, ""));

        instructionVector_free(instructions);
    }

    testVectorizeForm();
    testGenerateParseTree();
    testInstructionsEqual();
}

void testCodeGenerator() {
    void testIfStatement() {
        // Create a parse tree that represents the following piece of PL/0 code
        // and pass it to the code generator:
        //
        // int x;
        // begin
        //     read x
        //     if x = 0 then
        //         write x
        // end
        struct parseTree tree = pt(program
                (block
                    (var-declaration
                        (identifiers
                            (identifier x)))
                    (statement
                        (begin-block
                            (statements
                                (statement
                                    (read-statement
                                        (identifier x)))
                                (statments
                                    (statement
                                        (if-statement
                                            (condition
                                                (expression
                                                    (identifier x))
                                                (rel-op =)
                                                (expression
                                                    (number 0)))
                                            (statement
                                                (write-statement
                                                    (identifier 1)))))))))));

        // TODO: Free parse tree.
        
        struct instructionVector *instructions = generate(tree);
        // int x;
        // begin
        //     read x
        //     if x = 0 then
        //         write x
        // end
        assert(instructionsEqual(instructions,
                    "inc 0 1"   // Reserve space for int x
                    "sio 0 2"   // Read onto stack
                    "sto 0 0"   // Store read value in x
                    "lod 0 0"   // Load x onto stack
                    "lit 0 0"   // Push a 0 onto stack
                    "opr 0 8"   // Check for equality
                    "jpc 0 9"   // Jump to after if statement if comparison
                                // was false
                    "lit 0 1"   // Load a 1 onto stack
                    "sio 0 1"   // Output the 1
                    "opr 0 0"   // Return/Exit
                    ));

        instructionVector_free(instructions);
    }

    testIfStatement();
}

int main() {
    testTest();
    testCodeGenerator();

    printf("All tests passed.\n");

    return 0;
}
