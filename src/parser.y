%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>

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

%type<n> exp simp stmt stmts control block decl vars decls
%type<type> unop binop asop
%%
program : LEFTANG decls stmts RIGHTANG
	;

decls : decls decl
      {
	 $$ = $1;
	 $$.size += $2.size;
      }
      |
      {
	 $$.type = EDECL;
	 $$.size = 0;
      }
      ;

decl : VAR ID vars COLON type
     {
	$$ = $3;
	++$$.size;

	context_insert(&ctx, $2 + 1, *$2);
     }
     ;

vars : COMMA ID vars
     {
	$$ = $3;
	++$$.size; /* We only care about the count of variables */

	context_insert(&ctx, $2 + 1, *$2); /* We still wanna declare it */
     }
     |
     {
	$$.type = EDECL;
	$$.size = 0;
     }
     ;

type : INT /* Redundant rule since there is only one type */
     ;

stmts : stmts stmt
      {
	$$ = $1;
	if ($$.capacity == $$.size) {
	    $$.capacity *= 2;
	    $$.params = realloc($$.params, $$.capacity);
	    if (!$$.params) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	    }
	}
	$$.params[$$.size++] = $2;
      }
      |
      {
	$$.type = ECOMMA;
	$$.params = malloc(32 * sizeof(node));
	if (!$$.params) {
	    fprintf(stderr, "error: %s\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}
	$$.capacity = 32;
	$$.size = 0;
      }
      ;

stmt : simp { $$ = $1; }
     | control { $$ = $1; }
     | SEMICOLON { $$.type = ENOOP; }
     ;

simp : ID asop exp
     {
	$$.type = $2;
	$$.params = malloc(2 * sizeof(node));
	if (!$$.params) {
	    fprintf(stderr, "error: %s\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}
	$$.params[0].type = EID;
	$$.params[0].id = context_find(&ctx, $1 + 1, *$1);

	$$.params[1] = $3;
     }
     | PRINT exp
     {
	$$.type = EPRINT;
	$$.params = malloc(2 * sizeof(node));
	if (!$$.params) {
	    fprintf(stderr, "error: %s\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}
	$$.params[0] = $2;
     }
     ;

control : IF LEFTPAREN exp RIGHTPAREN block ELSE block %prec ELSE
	{
	    $$.type = EIF;
	    $$.params = malloc(3 * sizeof(node));
	    if (!$$.params) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	    }
	    $$.size = 3;
	    $$.params[0] = $3;
	    $$.params[1] = $5;
	    $$.params[2] = $7;
	}
	| IF LEFTPAREN exp RIGHTPAREN block %prec IF
	{
	    $$.type = EIF;
	    $$.params = malloc(2 * sizeof(node));
	    if (!$$.params) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	    }
	    $$.size = 2;
	    $$.params[0] = $3;
	    $$.params[1] = $5;
	}
	| WHILE LEFTPAREN exp RIGHTPAREN block
	{
	    $$.type = EWHILE;
	    $$.params = malloc(2 * sizeof(node));
	    if (!$$.params) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	    }
	    $$.params[0] = $3;
	    $$.params[1] = $5;
	}
	| FOR LEFTPAREN simp SEMICOLON exp SEMICOLON simp RIGHTPAREN block
	{
	    $$.type = EFOR;
	    $$.params = malloc(4 * sizeof(node));
	    if (!$$.params) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	    }
	    $$.params[0] = $3;
	    $$.params[1] = $5;
	    $$.params[2] = $7;
	    $$.params[3] = $9;
	}
	| CONTINUE SEMICOLON { $$.type = ECONTINUE; }
	| BREAK SEMICOLON { $$.type = EBREAK; }
	;

block : stmt { $$ = $1; }
      | LEFTANG stmts RIGHTANG { $$ = $2; }
      ;

exp : LEFTPAREN exp RIGHTPAREN { $$ = $2; }
    | NUM { $$.type = ELITERAL; $$.i = $1; }
    | ID { $$.type = EID; $$.id = context_find(&ctx, $1 + 1, *$1); }
    | unop exp %prec UNOP 
    {
	$$.type = $1;
	$$.params = malloc(sizeof(node));
	if (!$$.params) {
	    fprintf(stderr, "error: %s\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}
	*$$.params = $2;
    }
    | exp binop exp %prec BINOP
    {
	$$.type = $2;
	$$.params = malloc(2 * sizeof(node));
	if (!$$.params) {
	    fprintf(stderr, "error: %s\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}
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
    while (ids - ctx->ids < size && id_size != ids->size &&
						memcmp(id, ids->id, id_size)) {
	ids += ids->size;
    }

    if (!size) {
	return NULL;
    }

    return ids->id;
}

void
context_insert(context *ctx, const char *id, char id_size)
{
    if (ctx->capacity < ctx->size + id_size) {
	while (ctx->capacity < ctx->size + id_size) {
	    ctx->capacity *= 2;
	}

	ctx->ids = realloc(ctx->ids, ctx->capacity);
	if (!ctx->ids) {
	    fprintf(stderr, "error: %s\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}
	    
    }

    struct id *ids = ctx->ids + ctx->size;

    ids->size = id_size;
    memcpy(ids->id, id, id_size);
}
