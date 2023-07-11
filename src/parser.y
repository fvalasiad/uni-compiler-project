%{
#include	<stdio.h>

int yylex();
void yyerror(const char *);

%}

%%
program : /* empty */ { printf("Parsing complete!\n"); }
	;
%%
