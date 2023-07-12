%{
int yylex();
void yyerror(const char *, ...);
%}

%union {
    int i;
    char id[64]; /* A max of 63 characters ought to be enough for a variable name don't ya think? */
}

%token<i> NUM
%token<id> ID
%token VAR INT WHILE IF ELSE PRINT BREAK CONTINUE LEFTPAREN RIGHTPAREN FOR 
%token SEMICOLON LEFTANG RIGHTANG COLON COMMA

/* Lowest to highest. */
%right ASSIGN PLUSASSIGN SUBASSIGN MULASSIGN DIVASSIGN MODASSIGN 
%left OR
%left AND
%left EQ NOTEQ
%left LESS LESSEQ BIGGER BIGGEREQ
%left PLUS SUB
%left MUL DIV MOD
%right NOT
%left COMMA LEFTPAREN LEFTANG

/* Dangling else ambiguity */
%left IF
%left ELSE

/* Some additional dummy tokens with a set precedence to help yacc do its thing. */
%left BINOP
%right UNOP

%%
program : LEFTANG decls stmts RIGHTANG
	;

decls : decls decl
      |
      ;

decl : VAR ID vars COLON type
     ;

vars : COMMA ID vars
     |
     ;

type : INT
     ;

stmts : stmts stmt
      |

stmt : simp
     | control
     | SEMICOLON
     ;

simp : ID asop exp
     | PRINT exp
     ;

control : IF LEFTPAREN exp RIGHTPAREN block ELSE block %prec ELSE
	| IF LEFTPAREN exp RIGHTPAREN block %prec IF
	| WHILE LEFTPAREN exp RIGHTPAREN block
	| FOR LEFTPAREN simp SEMICOLON exp SEMICOLON simp RIGHTPAREN block
	| CONTINUE SEMICOLON
	| BREAK SEMICOLON
	;

block : stmt
      | LEFTANG stmts RIGHTANG
      ;

exp : LEFTPAREN exp RIGHTPAREN
    | NUM
    | ID
    | unop exp %prec UNOP
    | exp binop exp %prec BINOP
    ;

asop : ASSIGN
     | PLUSASSIGN
     | SUBASSIGN
     | MULASSIGN
     | DIVASSIGN
     | MODASSIGN
     ;

binop : OR
      | AND
      | EQ
      | NOTEQ
      | LESS
      | LESSEQ
      | BIGGER
      | BIGGEREQ
      | PLUS
      | SUB
      | MUL
      | DIV
      | MOD
      ;

unop : NOT
     | SUB %prec NOT 
     ;
%%
