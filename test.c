#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "generator.h"
#include "parser.h"
#include "vector.h"

// Turns the lisp-like form (a (b c) d) into a vector of strings containing
// "a", "(b c)", and "d".
struct vector *vectorizeForm(char *form) {
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

    struct vector *items = vector_init(sizeof(char*));
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
        vector_push(items, &item);

        c = end;
    }

    return items;
}

// Awesome alias for generateParseTree so that you don't have to type a bunch
// of quotes.
#define pt(forms) generateParseTree("(" #forms ")")

// Generates a parseTree struct from a string of lisp-like forms that
// represent the structure of the parse tree.
struct parseTree generateParseTree(char *forms) {
    struct vector *items = vectorizeForm(forms);

    //char *name = *(char**)vector_get(items, 0);
    char *name = (char*)vector_get(items, 0);

    if (strcmp(name, "identifier") == 0) {
        char *identifierName = (char*)vector_get(items, 1);
        struct vector *identifier = vector_init(sizeof(char*));
        vector_push(identifier, &identifierName);
        return (struct parseTree){"identifier", identifier};
    } else if (strcmp(name, "number") == 0) {
        char *numberString = (char*)vector_get(items, 1);
        int value = atoi(numberString);
        struct vector *number = vector_init(sizeof(int));
        vector_push(number, &value);
        return (struct parseTree){"number", number};
    }

    struct vector *children = (struct vector*)vector_init(sizeof(struct parseTree));
    int i;
    for (i = 1; i < items->length; i++) {
        char *item = *(char**)vector_get(items, i);
        struct parseTree childTree = generateParseTree(item);
        vector_push(children, &childTree);
    }

    return (struct parseTree){name, children};
}

int opcodesEqual(struct vector *opcodes, char *expectedCode) {
    return 1;
}

// Test the code that was defined in this file to make testing easier.
void testTest() {
    struct vector *items = vectorizeForm("(aaa (bbb ccc) ddd)");
    assert(strcmp(*(char**)vector_get(items, 0), "aaa") == 0);
    assert(strcmp(*(char**)vector_get(items, 1), "(bbb ccc)") == 0);
    assert(strcmp(*(char**)vector_get(items, 2), "ddd") == 0);

    struct parseTree tree = pt(aaa (identifier ccc) ddd);
    assert(strcmp(tree.name, "aaa") == 0);
    struct parseTree child = *(struct parseTree*)vector_get(tree.children, 0);
    printf("%s\n", child.name);
    assert(strcmp(child.name, "identifier") == 0);
    struct parseTree grandchild = *(struct parseTree*)vector_get(child.children, 0);
    printf("%s\n", grandchild.name);
    assert(strcmp(grandchild.name, "ccc") == 0);
    struct parseTree child2 = *(struct parseTree*)vector_get(tree.children, 1);
    assert(strcmp(child2.name, "ddd") == 0);
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
        
        struct vector *opcodes = generate(tree);
        // int x;
        // begin
        //     read x
        //     if x = 0 then
        //         write x
        // end
        assert(opcodesEqual(opcodes,
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
    }

    testIfStatement();
}

int main() {
    testTest();
    testCodeGenerator();
}
