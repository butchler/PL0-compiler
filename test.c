#include <assert.h>
#include "generator.h"
#include "parser.h"
#include "vector.h"

void testCodeGenerator() {
    // Turns the lisp-like form (a (b c) d) into a vector of strings containing
    // "a", "(b c)", and "d".
    struct vector *vectorizeForm(char *form) {
        char *formStart = index(form, '(');
        char *formEnd = rindex(form, ')');

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

        struct vector *items = vector_init(sizeof(char*));
        char *c = formStart + 1;
        while (c < formEnd) {

            char *start = skipWhitespace(c);

            char *end = NULL;
            if (*start == '(')
                end = skipForm(start);
            else
                end = skipNonwhitespace(start);
            if (end > formEnd)
                end = formEnd;

            int length = end - start;
            char *item = strndup(start, length);
            vector_push(items, item);

            c = end;
        }

        return items;
    }

    // Generates a parseTree struct from a string of lisp-like forms that
    // represent the structure of the parse tree.
    struct parseTree pt(char *forms) {
        struct vector *items = vectorizeForm(forms);

        char *name = (char*)vector_get(items, 0);

        if (strcmp(name, "identifier") == 0) {
            char *identiferName = (char*)vector_get(items, 1);
            return (struct parseTree){"identifier", NULL,  identifierName};
        } else if (strcmp(name, "number") == 0) {
            char *number = (char*)vector_get(items, 1);
            return (struct parseTree){"number", NULL, number};
        }

        struct parseTree parseTree = {name, NULL, NULL};
        struct vector *children = (struct vector*)vector_init(sizeof(struct parseTree));
        int i;
        for (i = 1; i < items.length; i++) {
            char *item = (char*)vector_get(items, i);
            vector_push(children, pt(item));
        }
        parseTree.children = children;

        return parseTree;
    }

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
        struct parseTree parseTree = pt("\
            (program\
                (block\
                    (var-declaration\
                        (identifiers\
                            (identifier x)))\
                    (statement\
                        (begin-block\
                            (statements\
                                (statement\
                                    (read-statement\
                                        (identifier x)))\
                                (statments\
                                    (statement\
                                        (if-statement\
                                            (condition\
                                                (expression\
                                                    (identifier x))\
                                                (rel-op =)\
                                                (expression\
                                                    (number 0)))\
                                            (statement\
                                                (write-statement\
                                                    (identifier 1)))))))))))");
        
        struct parseTree parseTree = pt("program",
                pt("block",
                    pt("var-declaration",
                        pt("identifiers",
                            pt("identifier", "x")))
                    pt("statement",
                        pt("begin-block",
                            pt("statements",
                                pt("statement",
                                    pt("read-statement",
                                        pt("identifier", "x"))),
                                pt("statments",
                                    pt("statement",
                                        pt("if-statement",
                                            pt("condition",
                                                pt("expression",
                                                    pt("identifier", "x")),
                                                pt("rel-op", "="),
                                                pt("expression",
                                                    pt("number", "0"))),
                                            pt("statement",
                                                pt("write-statement",
                                                    pt("identifier", "1")))))))))));
        struct vector opcodes = generate(parseTree);
        // int x;
        // begin
        //     read x
        //     if x = 0 then
        //         write x
        // end
        assert(opcodesEqual(opcodes,
                    "inc", 0, 1,   // Reserve space for int x
                    "sio", 0, 2,   // Read onto stack
                    "sto", 0, 0,   // Store read value in x
                    "lod", 0, 0,   // Load x onto stack
                    "lit", 0, 0,   // Push a 0 onto stack
                    "opr", 0, 8,   // Check for equality
                    "jpc", 0, 9,   // Jump to after if statement if comparison
                                   // was false
                    "lit", 0, 1,   // Load a 1 onto stack
                    "sio", 0, 1,   // Output the 1
                    "opr", 0, 0    // Return/Exit
                    ));
    }
}

int main() {
    testCodeGenerator();
}
