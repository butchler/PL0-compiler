digit [0-9]
identifier [a-zA-Z][a-zA-Z0-9]*
comment [/][*]([^*]|[*]+[^*/])*[*]+[/]

%%

{digit}+        printf("number-token (%s) ", yytext);
{comment}       /* Throw away comments. */
begin|while|const|write|call|then|procedure|read|else|odd|end|int|if|do|>=|<=|<>|:=|"+"|"-"|"*"|"/"|=|<|>|"("|")"|,|;|"." printf("%s ", yytext);
{identifier}    printf("identifier-token (%s) ", yytext);

%%

int main() {
    yylex();
}
