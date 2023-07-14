%{
/* The limitations of POSIX yacc forces us to rely on include order,
unfortunately. */
#include "types.h"
#include "parser.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void yyerror(const char *, ...);

int yywrap();
%}

letter	[a-zA-Z]
digit	[0-9]

id	{letter}[{letter}{digit}_]*
num	-?[1-9]{digit}*

%%

{id}	{
    if (yyleng >= sizeof(yylval.id)) {
	yyerror("Consider using variables of length less than %d\n", sizeof(yylval.id));
    }
    yylval.id[0] = yyleng;

    memcpy(yylval.id + 1, yytext, yyleng); /* Also copy the zero terminating byte. */
    return ID;
}
{num}	{
    /* Uncertain if the requirements state that I should check for the
       integer's valid range here, probably I guess? */
    errno = 0;
    yylval.i = strtol(yytext, NULL, 10);
    if (errno == ERANGE) {
	yyerror("Up to 4 bytes, respect the limits.");
    } else if (errno) {
	fprintf(stderr, "Error trying to parse a num: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }
    return NUM;
}
"var"	{return VAR;}
"int"	{return INT;}
"while"	{return WHILE;}
"if"	{return IF;}
"else"	{return ELSE;}
"print"	{return PRINT;}
"break"	{return BREAK;}
"continue"	{return CONTINUE;}
":"	{return COLON;}
","	{return COMMA;}
"("	{return LEFTPAREN;}
")"	{return RIGHTPAREN;}
"for"	{return FOR;}
";"	{return SEMICOLON;}
"{"	{return LEFTANG;}
"}"	{return RIGHTANG;}
"!"	{return NOT;}
"-"	{return SUB;}
"*"	{return MUL;}
"/"	{return DIV;}
"%"	{return MOD;}
"+"	{return PLUS;}
"<"	{return LESS;}
"<="	{return LESSEQ;}
">"	{return BIGGER;}
">="	{return BIGGEREQ;}
"=="	{return EQ;}
"!="	{return NOTEQ;}
"&&"	{return AND;}
"||"	{return OR;}
"="	{return ASSIGN;}
"+="	{return PLUSASSIGN;}
"-="	{return SUBASSIGN;}
"*="	{return MULASSIGN;}
"/="	{return DIVASSIGN;}
"%="	{return MODASSIGN;}
[ \t\n] {}
.	{ yyerror("unrecognized token\n"); }

%%
