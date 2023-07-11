%{
#include	<stdio.h>

void yyerror(const char *);

int yywrap();

%}

%%
.|\n	{ printf("Found: %s\n", yytext); }
%%
