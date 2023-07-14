%{
#include <string.h>
#include <stddef.h>

#include "types.h"

context ctx;

int yylex();
void yyerror(const char *, ...);
%}

%union {
    int i;
    char id[64]; /* A max of 63 characters ought to be enough for a variable name don't ya think? */
    node n;
    node_type type;
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

%type<n> exp
%type<type> unop binop asop

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

exp : LEFTPAREN exp RIGHTPAREN { $$ = $2; }
    | NUM { $$.type = ELITERAL; $$.i = $1; }
    | ID { $$.type = EID; $$.id = context_find(&ctx, $1 + 1, *$1); }
    | unop exp %prec UNOP 
    {
	$$.type = $1;
	$$.params = malloc(sizeof(node));
	*$$.params = $2;
    }
    | exp binop exp %prec BINOP
    {
	$$.type = $2;
	$$.params = malloc(2 * sizeof(node));
	$$.params[0] = $1; $$.params[1];
    }
    ;

asop : ASSIGN { $$ = EASSIGN; }
     | PLUSASSIGN { $$ = EPLUSASSIGN; }
     | SUBASSIGN { $$ = ESUBASSIGN; }
     | MULASSIGN { $$ = EMULASSIGN; }
     | DIVASSIGN { $$ = EDIVASSIGN; }
     | MODASSIGN { $$ = EMODASSIGN; }
     ;

binop : OR { $$ = EOR; }
      | AND { $$ = EAND; }
      | EQ { $$ = EEQ; }
      | NOTEQ { $$ = ENOTEQ; }
      | LESS { $$ = ELESS; }
      | LESSEQ { $$ = ELESSEQ; }
      | BIGGER { $$ = EBIGGER; }
      | BIGGEREQ { $$ = EBIGGEREQ; }
      | PLUS { $$ = EPLUS; }
      | SUB { $$ = ESUB; }
      | MUL { $$ = EMUL; }
      | DIV { $$ = EDIV; }
      | MOD { $$ = EMOD; }
      ;

unop : NOT { $$ = ENOT; }
     | SUB %prec NOT { $$ = EUMINUS; }
     ;
%%

char *
context_find(const context *ctx, const char *id, char id_size)
{
    int size = ctx->size;
    struct id *ids = ctx->ids;

    /* There are size ids registered. */
    while (size-- && id_size != ids->size && memcmp(id, ids->id, id_size)) {
	ids += ids->size;
    }

    if (!size) {
	return NULL;
    }

    return ids->id;
}
