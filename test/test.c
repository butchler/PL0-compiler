#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "test/lib/parser.h"
#include "test/lib/generator.h"

#define assertf(a) { assert(a);printf(".");}
#define test(fn) { printf("" #fn ":\n\t"); fn();printf("\n");}

void testTestUtil() {
    void testVectorizeForm() {
        struct vector *items = formToVector("(aaa (bbb ccc) ddd)");

        assertf(strcmp(get(char*, items, 0), "aaa") == 0);
        assertf(strcmp(get(char*, items, 1), "(bbb ccc)") == 0);
        assertf(strcmp(get(char*, items, 2), "ddd") == 0);

        freeVector(items);
    }

    void testGenerateParseTree() {
        struct parseTree tree = pt(aaa (bbb (identifier ccc)) (number 123));
        assertf(strcmp(tree.name, "aaa") == 0);
        struct parseTree child = get(struct parseTree, tree.children, 0);
        assertf(strcmp(child.name, "bbb") == 0);
        struct parseTree grandchild = get(struct parseTree, child.children, 0);
        assertf(strcmp(grandchild.name, "identifier") == 0);
        char *identifier = get(struct parseTree, grandchild.children, 0).name;
        assertf(strcmp(identifier, "ccc") == 0);
        struct parseTree child2 = get(struct parseTree, tree.children, 1);
        assertf(strcmp(child2.name, "number") == 0);

        // TODO: Free parse tree
    }

    void testInstructionsEqual() {
        struct vector *instructions = makeVector(struct instruction);
        struct instruction instruction;
        instruction = (struct instruction){1, 0, 0}; push(instructions, instruction);
        instruction = (struct instruction){2, 0, 0}; push(instructions, instruction);
        instruction = (struct instruction){3, 0, 0}; push(instructions, instruction);
        assertf(instructionsEqual(instructions,
                    " lit 0 0"
                    " opr 0 0"
                    " lod 0 0"));
        assertf(!instructionsEqual(instructions, ""));

        freeVector(instructions);
    }

    test(testVectorizeForm);
    test(testGenerateParseTree);
    test(testInstructionsEqual);
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
                                (statements
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
                                                    (identifier x)))))))))));

        // TODO: Free parse tree.
        
        struct vector *instructions = generateInstructions(tree);
        // int x;
        // begin
        //     read x
        //     if x = 0 then
        //         write x
        // end
        if (instructions == NULL)
            puts(getGeneratorError());

        assertf(instructions != NULL);
        assertf(instructionsEqual(instructions,
                    " inc 0 1"   // Reserve space for int x
                    " sio 0 2"   // Read onto stack
                    " sto 0 0"   // Store read value in x
                    " lod 0 0"   // Load x onto stack
                    " lit 0 0"   // Push a 0 onto stack
                    " opr 0 8"   // Check for equality
                    " jpc 0 9"   // Jump to after if statement if comparison
                                 // was false
                    " lod 0 0"   // Load x onto the stack
                    " sio 0 1"   // Output the value of x
                    " opr 0 0"   // Return/Exit
                    ));

        freeVector(instructions);
    }

    testIfStatement();
}

int main() {
    test(testTestUtil);
    test(testCodeGenerator);

    printf("\nAll tests passed.\n");

    return 0;
}
