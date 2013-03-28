#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "src/lexer.h"
#include "test/lib/parser.h"
#include "test/lib/generator.h"

void testTestUtil() {
    void testVectorizeForm() {
        struct vector *items = formToVector("(aaa (bbb ccc) ddd)");

        assert(strcmp(get(char*, items, 0), "aaa") == 0);
        assert(strcmp(get(char*, items, 1), "(bbb ccc)") == 0);
        assert(strcmp(get(char*, items, 2), "ddd") == 0);

        freeVector(items);
    }

    void testGenerateParseTree() {
        struct parseTree tree = pt(aaa (bbb (identifier ccc)) (number 123));
        assert(strcmp(tree.name, "aaa") == 0);
        struct parseTree child = get(struct parseTree, tree.children, 0);
        assert(strcmp(child.name, "bbb") == 0);
        struct parseTree grandchild = get(struct parseTree, child.children, 0);
        assert(strcmp(grandchild.name, "identifier") == 0);
        char *identifier = get(struct parseTree, grandchild.children, 0).name;
        assert(strcmp(identifier, "ccc") == 0);
        struct parseTree child2 = get(struct parseTree, tree.children, 1);
        assert(strcmp(child2.name, "number") == 0);

        // TODO: Free parse tree.
    }

    void testInstructionsEqual() {
        struct vector *instructions = makeVector(struct instruction);
        struct instruction instruction;
        instruction = (struct instruction){1, 0, 0}; push(instructions, instruction);
        instruction = (struct instruction){2, 0, 0}; push(instructions, instruction);
        instruction = (struct instruction){3, 0, 0}; push(instructions, instruction);
        assert(instructionsEqual(instructions,
                    " lit 0 0"
                    " opr 0 0"
                    " lod 0 0"));
        assert(!instructionsEqual(instructions, ""));

        freeVector(instructions);
    }

    testVectorizeForm();
    testGenerateParseTree();
    testInstructionsEqual();
}

void testLexer() {
    initLexer();

    struct vector *lexemes = readLexemes(
            "int x;\n"
            "begin\n"
                "read x\n"
                "if x = 0 then\n"
                    "write x\n"
            "end\n");

    /*int i;
    for (i = 0; i < lexemes->length; i++) {
        struct lexeme lexeme = get(struct lexeme, lexemes, i);
        printf("%s ", lexeme.token);
    }
    printf("\n");*/
}

void testParser() {

    void addRule(struct grammar grammar, char *variable, char *constProductionString) {
        char *productionString = strdup(constProductionString);
        struct vector *production = makeVector(char*);
        while (productionString != NULL) {
            char *varOrTerminal = strsep(&productionString, " ");
            push(production, varOrTerminal);
        }
        pushLiteral(grammar.rules, struct rule, {variable, production});
    }

    void printParseTree(struct parseTree tree, int level) {
        void printIndent() {
            int i;
            for (i = 0; i < 4 * level; i++)
                printf(" ");
        }

        printIndent();
        printf("%s\n", tree.name);

        if (tree.children != NULL) {
            int i;
            for (i = 0; i < tree.children->length; i++) {
                struct parseTree child = get(struct parseTree, tree.children, i);
                printParseTree(child, level + 1);
            }
        }
    }

    //initLexer();

    struct vector *lexemes = readLexemes("int x;");

    struct grammar grammar = {makeVector(struct rule)};
    addRule(grammar, "integer", "intsym identifier semicolonsym");
    struct parseTree tree = parse(lexemes, 0, "integer", grammar);
    assert(tree.name != NULL);
    printParseTree(tree, 0);

    lexemes = readLexemes(
            "begin\n"
                "read x\n"
                "write x\n"
            "end\n");

    grammar = (struct grammar){makeVector(struct rule)};
    addRule(grammar, "begin-block", "beginsym statements endsym");
    addRule(grammar, "statements", "statement");
    addRule(grammar, "statements", "statement statements");
    addRule(grammar, "statement", "read-statement");
    addRule(grammar, "statement", "write-statement");
    addRule(grammar, "read-statement", "identifier readsym");
    addRule(grammar, "write-statement", "identifier writesym");
    tree = parse(lexemes, 0, "begin-block", grammar);
    assert(tree.name != NULL);
    printParseTree(tree, 0);
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
        assert(instructions != NULL);
        assert(instructionsEqual(instructions,
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
    testTestUtil();
    testLexer();
    testParser();
    testCodeGenerator();

    printf("All tests passed.\n");

    return 0;
}
