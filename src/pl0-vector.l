%{
#include "src/lib/vector.h"
#include "src/lexer.h"
#include "src/pl0.h"
struct vector *pl0Tokens;
char *pl0Source;
void addToken(char *type, char *token) {
    pushLiteral(pl0Tokens, struct lexeme, {strdup(type), strdup(token)});
}
#define ECHO /* Stop the generated lexer code from outputing anything. */
/* Redefine YY_INPUT to read from the string passed to readPL0Tokens. */
#define YY_INPUT(buf, result, max_size)\
{\
    int length = strlen(pl0Source);\
    if (length == 0) {\
        result = YY_NULL;\
    } else if (length < max_size) {\
        memcpy(buf, pl0Source, length);\
        result = length;\
        pl0Source += result;\
    } else {\
        memcpy(buf, pl0Source, max_size);\
        result = max_size;\
        pl0Source += result;\
    }\
}
%}

digit [0-9]
identifier [a-zA-Z][a-zA-Z0-9]*
comment [/][*]([^*]|[*]+[^*/])*[*]+[/]

%%
{digit}+        addToken("number-token", yytext);
{comment}       /* Ignore comments. */
begin|while|const|write|call|then|procedure|read|else|odd|end|int|if|do|>=|<=|<>|:=|"+"|"-"|"*"|"/"|=|<|>|"("|")"|,|;|"." addToken(yytext, yytext);
{identifier}    addToken("identifier-token", yytext);
%%

struct vector *readPL0Tokens(char *source) {
    pl0Source = source;
    pl0Tokens = makeVector(struct lexeme);

    yylex();

    return pl0Tokens;
}
