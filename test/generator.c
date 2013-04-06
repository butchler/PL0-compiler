#include "test/test.h"

void testCodeGenerator() {
    /*void testInstructionsEqual() {
        struct vector *instructions = makeVector(struct instruction);
        struct instruction instruction;
        instruction = (struct instruction){1, 0, 0}; push(instructions, instruction);
        instruction = (struct instruction){2, 0, 0}; push(instructions, instruction);
        instruction = (struct instruction){3, 0, 0}; push(instructions, instruction);
        assert(instructionsEqual(instructions,
                    "lit 0 0,"
                    "opr 0 0,"
                    "lod 0 0"));
        assert(!instructionsEqual(instructions, ""));

        freeVector(instructions);
    }

    testInstructionsEqual();

    void testIfStatement() {
        // Create a parse tree that represents the following piece of PL/0 code
        // and pass it to the code generator:
        //
        // const y = 3;
        // int x;
        // begin
        //     read x
        //     if x = y then
        //         write x
        // end.
        struct parseTree tree = pt(program
                (block
                    (var-declaration
                        (vars
                            (var (identifier x))))
                    (const-declaration
                        (constants
                            (constant (identifier y) (number 3))))
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
                                                    (term (factor (identifier x))))
                                                (rel-op =)
                                                (expression
                                                    (term (factor (identifier y)))))
                                            (statement
                                                (write-statement
                                                    (identifier x)))))))))));

        struct vector *instructions = generateInstructions(tree);
        if (generatorHasErrors())
            printGeneratorErrors();
        assert(instructions != NULL);
        assert(instructionsEqual(instructions,
                    " inc 0 1"   // Reserve space for int x
                    " sio 0 2"   // Read onto stack
                    " sto 0 0"   // Store read value in x
                    " lod 0 0"   // Load x onto stack
                    " lit 0 3"   // Push the value of y onto stack
                    " opr 0 8"   // Check for equality
                    " jpc 0 9"   // Jump to after if statement if comparison
                                 // was false
                    " lod 0 0"   // Load x onto the stack
                    " sio 0 1"   // Output the value of x
                    " opr 0 0"   // Return/Exit
                    ));

        freeParseTree(tree);
        freeVector(instructions);
    }

    void testExpression() {
        initLexer();

        // Define grammar.
        struct grammar grammar = {makeVector(struct rule)};
        addRule(grammar, "expression", "term add-or-subtract expression");
        addRule(grammar, "expression", "term");
        addRule(grammar, "add-or-subtract", "plussym");
        addRule(grammar, "add-or-subtract", "minussym");
        addRule(grammar, "term", "factor multiply-or-divide term");
        addRule(grammar, "term", "factor");
        addRule(grammar, "multiply-or-divide", "multsym");
        addRule(grammar, "multiply-or-divide", "slashsym");
        addRule(grammar, "factor", "lparentsym expression rparentsym");
        addRule(grammar, "factor", "sign number");
        addRule(grammar, "factor", "identifier");
        addRule(grammar, "sign", "plussym");
        addRule(grammar, "sign", "minussym");
        addRule(grammar, "sign", "nothing");
        addRule(grammar, "number", "numbersym");

        int expressionBecomes(char *expression, char *expectedInstructions) {
            // Read tokens.
            struct vector *lexemes = readLexemes(expression);
            // Parse tokens.
            struct parseTree tree = parse(lexemes, 0, "expression", grammar);
            // Generate code.
            struct vector *instructions = generateInstructions(tree);
            if (generatorHasErrors())
                printGeneratorErrors();
            assert(instructions != NULL);
            // Test generated code.
            return instructionsEqual(instructions, expectedInstructions);
        }

        assert(expressionBecomes("5", "lit 0 5"));
        assert(expressionBecomes("-100", "lit 0 100, opr 0 1"));
        assert(expressionBecomes("5 + 10", "lit 0 5, lit 0 10, opr 0 2"));
        assert(expressionBecomes("1 + 2 + 3", "lit 0 1, lit 0 2, lit 0 3, opr 0 2, opr 0 2"));
        assert(expressionBecomes("-3 * (5 + -10)",
                    "lit 0 3,"
                    "opr 0 1,"
                    "lit 0 5,"
                    "lit 0 10,"
                    "opr 0 1,"
                    "opr 0 2,"
                    "opr 0 4"));
    }

    testIfStatement();
    testExpression();*/
}
