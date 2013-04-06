#include "src/pl0.h"
#include "src/lexer.h"
#include "src/parser.h"
#include "src/lib/vector.h"
#include <stdlib.h>
#include <string.h>

struct vector *getPL0Tokens() {
    // Regexes that match each token type, along with the token type that they
    // map to. The token type strings are the strings used in the PL/0 grammar.
    // The regexes are compiled with REG_EXTENDED (they are interpreted as
    // POSIX extended regular expressions).
    struct tokenDefinition tokenDefinitions[] = {

        // Whitespace
        {"\\s+", "whitespace"},
        // The comment regex is based off of http://ostermiller.org/findcomment.html
        // [*] matches a single * character, and looks slightely nicer than \\*
        // (\\* instead of \* because it needs to be escaped twice: once for the C
        // string and another time for the regex.)
        {"/[*]([^*]|[*]+[^*/])*[*]+/", "comment"},
        // Keywords
        // \\b = \b and matches the empty string, but only along a word boundary,
        // where words anything that contains [a-zA-Z0-9_].
        {"begin\\b", "begin"},
        {"while\\b", "while"},
        {"const\\b", "const"},
        {"write\\b", "write"},
        {"call\\b", "call"},
        {"then\\b", "then"},
        {"procedure\\b", "procedure"},
        {"read\\b", "read"},
        {"else\\b", "else"},
        {"odd\\b", "odd"},
        {"end\\b", "end"},
        {"int\\b", "int"},
        {"if\\b", "if"},
        {"do\\b", "do"},
        // Identifiers
        {"[a-zA-Z][a-zA-Z]*", "identifier-token"},
        // Numbers
        {"[0-9]+", "number-token"},
        // Special symbols
        {">=", ">="},
        {"<=", "<="},
        {"<>", "<>"},
        {":=", ":="},
        {"[+]", "+"},
        {"-", "-"},
        {"[*]", "*"},
        {"/", "/"},
        {"=", "="},
        {"<", "<"},
        {">", ">"},
        {"[(]", "("},
        {"[)]", ")"},
        {",", ","},
        {";", ";"},
        {"[.]", "."},
        // Indicates the end of token definitions.
        {NULL, NULL}

    };

    // Convert the above array into a vector.
    struct vector *tokenDefs = makeVector(struct tokenDefinition);
    int i;
    for (i = 0; tokenDefinitions[i].regexString != NULL; i++) {
        push(tokenDefs, tokenDefinitions[i]);
    }

    return tokenDefs;
}

struct grammar getPL0Grammar() {
    // Define full PL/0 grammar
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

struct vector *removeWhitespaceAndComments(struct vector *lexemes) {
    struct vector *newLexemes = makeVector(struct lexeme);

    forVector(lexemes, i, struct lexeme, lexeme,
            if (strcmp(lexeme.tokenType, "whitespace") != 0
                && strcmp(lexeme.tokenType, "comment") != 0)
                push(newLexemes, lexeme););

    return newLexemes;
}

