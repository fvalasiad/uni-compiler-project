%{
void yyerror(const char *);

int yywrap();

%}

letter	[a-zA-Z]
digit	[0-9]

id	{letter}[{letter}{digit}_]*
num	-?[1-9]{digit}*

%%

{id}	{}
{num}	{}
"var"	{}
"int"	{}
"while"	{}
"if"	{}
"else"	{}
"print"	{}
"break"	{}
[ \t\n] {}
"!"	{}
"-"	{}
"*"	{}
"/"	{}
"%"	{}
"+"	{}
"<"	{}
"<="	{}
">"	{}
">="	{}
"=="	{}
"!="	{}
"&&"	{}
"||"	{}
"="	{}
"+="	{}
"-="	{}
"*="	{}
"/="	{}
"%="	{}
.	{ yyerror("unrecognized token\n"); }

%%
