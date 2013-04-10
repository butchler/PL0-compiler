#include "lib/pl0.h"
#include "lib/lexer.h"
#include "lib/parser.h"
#include "lib/vector.h"
#include <stdlib.h>
#include <string.h>

// readPL0Tokens() is defined in pl0-lexer.c, which is generated from
// pl0-vector.l by flex.

struct grammar getPL0Grammar() {
    // Define full PL/0 grammar.
    // The @'s before the variable names don't have any special meaning,
    // they're just a convention I'm using to make it easier to know what's a
    // variable and what's a terminal in the production rules.
    struct grammar grammar = (struct grammar){makeVector(struct rule)};
    addRule(grammar, "program", "@block .");

    addRule(grammar, "@block", "@const-declaration @var-declaration @statement");

    addRule(grammar, "@const-declaration", "const @constants ;");
    addRule(grammar, "@const-declaration", "nothing");
    addRule(grammar, "@constants", "@constant , @constants");
    addRule(grammar, "@constants", "@constant");
    addRule(grammar, "@constant", "@identifier = @number");

    addRule(grammar, "@var-declaration", "int @vars ;");
    addRule(grammar, "@var-declaration", "nothing");
    addRule(grammar, "@vars", "@var , @vars");
    addRule(grammar, "@vars", "@var");
    addRule(grammar, "@var", "@identifier");

    addRule(grammar, "@statement", "@read-statement");
    addRule(grammar, "@statement", "@write-statement");
    addRule(grammar, "@statement", "@assignment");
    addRule(grammar, "@statement", "@if-statement");
    addRule(grammar, "@statement", "@while-statement");
    addRule(grammar, "@statement", "@begin-block");
    addRule(grammar, "@statement", "nothing");

    addRule(grammar, "@assignment", "@identifier := @expression");

    addRule(grammar, "@begin-block", "begin @statements end");
    addRule(grammar, "@statements", "@statement ; @statements");
    addRule(grammar, "@statements", "@statement");

    addRule(grammar, "@if-statement", "if @condition then @statement");
    addRule(grammar, "@condition", "@expression @rel-op @expression");
    addRule(grammar, "@condition", "odd @expression");
    addRule(grammar, "@rel-op", "=");
    addRule(grammar, "@rel-op", "<>");
    addRule(grammar, "@rel-op", "<");
    addRule(grammar, "@rel-op", "<=");
    addRule(grammar, "@rel-op", ">");
    addRule(grammar, "@rel-op", ">=");

    // This is the grammar for expressions included in the assignment.
    /*addRule(grammar, "@expression", "@sign @term @add-or-substract @term");
    addRule(grammar, "@expression", "@sign @term");
    addRule(grammar, "@sign", "+");
    addRule(grammar, "@sign", "-");
    addRule(grammar, "@sign", "nothing");
    addRule(grammar, "@add-or-substract", "+");
    addRule(grammar, "@add-or-substract", "-");
    addRule(grammar, "@term", "@factor @multiply-or-divide @factor");
    addRule(grammar, "@term", "@factor");
    addRule(grammar, "@multiply-or-divide", "*");
    addRule(grammar, "@multiply-or-divide", "/");
    addRule(grammar, "@factor", "( @expression )");
    addRule(grammar, "@factor", "@identifier");
    addRule(grammar, "@factor", "@number");*/

    // This is an improved grammar for expressions that behaves more like you
    // would intuitively expeted expressions to behave. For example, 1 + 2 + 3
    // is not a valid expression in the other grammar, you would have to do
    // something like 1 + (2 + 3) instead, but it works with this grammar.
    addRule(grammar, "@expression", "@term @add-or-subtract @expression");
    addRule(grammar, "@expression", "@term");
    addRule(grammar, "@add-or-subtract", "+");
    addRule(grammar, "@add-or-subtract", "-");
    addRule(grammar, "@term", "@factor @multiply-or-divide @term");
    addRule(grammar, "@term", "@factor");
    addRule(grammar, "@multiply-or-divide", "*");
    addRule(grammar, "@multiply-or-divide", "/");
    addRule(grammar, "@factor", "( @expression )");
    addRule(grammar, "@factor", "@sign @number");
    addRule(grammar, "@factor", "@identifier");
    addRule(grammar, "@sign", "+");
    addRule(grammar, "@sign", "-");
    addRule(grammar, "@sign", "nothing");

    addRule(grammar, "@while-statement", "while @condition do @statement");

    addRule(grammar, "@read-statement", "read @identifier");
    addRule(grammar, "@write-statement", "write @identifier");

    addRule(grammar, "@identifier", "identifier-token");
    addRule(grammar, "@number", "number-token");

    return grammar;
}

struct parseTree parsePL0Tokens(struct vector *tokens) {
    return parse(tokens, getPL0Grammar(), "program");
}

