/*
Copyright (C) 2023 Fotios Valasiadis
SPDX-License-Identifier: GPL-3.0-or-later
*/

%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>

#include "types.h"

extern context ctx;

extern int yycolumn;
extern int yylineno;

int yylex();
void yyerror(const char *, ...);
%}

%locations

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

%start program

%%
program : LEFTANG decls stmts RIGHTANG
	{
	    ctx.tree.type = ECOMMA;
	    ctx.tree.params = malloc(2 * sizeof(node));
	    if (!ctx.tree.params) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	    }

	    ctx.tree.params[0] = $2;
	    ctx.tree.params[1] = $3;
	    ctx.tree.size = 2;
	}
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
	$$.type = EASSIGN;
	$$.params = malloc(2 * sizeof(node));
	if (!$$.params) {
	    fprintf(stderr, "error: %s\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}

	$$.params[0].type = EID;
	$$.params[0].i = context_find(&ctx, $1 + 1, *$1);

	if ($2 != EASSIGN) {
	    $$.params[1].type = $2;
	    $$.params[1].params = malloc(2 * sizeof(node));
	    if (!$$.params[1].params) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	    }
	    $$.params[1].params[0] = $$.params[0];
	    $$.params[1].params[1] = $3;
	} else {
	    $$.params[1] = $3;
	}
     }
     | PRINT exp
     {
	$$.type = EPRINT;
	$$.params = malloc(sizeof(node));
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
    | ID { $$.type = EID; $$.i = context_find(&ctx, $1 + 1, *$1); }
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
	$$.params[0] = $1;
	$$.params[1] = $3;
    }
    ;

asop : ASSIGN { $$ = EASSIGN; }
     | PLUSASSIGN { $$ = EPLUS; }
     | SUBASSIGN { $$ = ESUB; }
     | MULASSIGN { $$ = EMUL; }
     | DIVASSIGN { $$ = EDIV; }
     | MODASSIGN { $$ = EMOD; }
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

#include <stdarg.h>

extern int yyleng;

void
yyerror(const char *format, ...)
{
    fprintf(stderr, "%d:%d: error:", yylineno, yycolumn - yyleng);

    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");
}

int
context_find(const context *ctx, const char *id, char id_size)
{
    int size = ctx->size;
    struct id *ids = ctx->ids;
    int count = 0;
    /* There are size ids registered. */
    while (ids - ctx->ids < size && !(id_size == ids->size &&
						!memcmp(id, ids->id, id_size))) {
	ids += (ids->size + sizeof(struct id));
	++count;
    }

    if (ids - ctx->ids >= size) {
	return -1;
    }

    return count;
}

void
context_insert(context *ctx, const char *id, char id_size)
{
    int found = context_find(ctx, id, id_size);
    if (found != -1) {
	fprintf(stderr, "error: redeclaration of variable ");
	for (int i = 0; i < id_size; ++i) {
	    putc(id[i], stderr);
	}
	putc('\n', stderr);
    }

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
    ctx->size += id_size + sizeof(struct id);

    ids->size = id_size;
    memcpy(ids->id, id, id_size);
}

static void
print_recurse(node *n, int depth, FILE *out)
{
    for (int i = 0; i < depth; ++i) fprintf(out, "\t");
    int count;
#define lazy(t, c) case E##t: fprintf(out, "%s\n", #t); count = c; break
    switch(n->type) {
	case ELITERAL: fprintf(out, "LITERAL %d\n", n->i); count = 0; break;
	lazy(NOT, 1);
	lazy(UMINUS, 1);
	lazy(MOD, 2);
	lazy(DIV, 2);
	lazy(MUL, 2);
	lazy(SUB, 2);
	lazy(PLUS, 2);
	lazy(BIGGEREQ, 2);
	lazy(BIGGER, 2);
	lazy(LESSEQ, 2);
	lazy(LESS, 2);
	lazy(NOTEQ, 2);
	lazy(EQ, 2);
	lazy(AND, 2);
	lazy(OR, 2);
	lazy(ASSIGN, 2);
	case EID: fprintf(out, "ID %d\n", n->i); count = 0; break;
	lazy(PRINT, 1);
	lazy(NOOP, 0);
	lazy(BREAK, 0);
	lazy(CONTINUE, 0);
	lazy(COMMA, n->size);
	case EDECL: fprintf(out, "DECL %d\n", n->size); count = 0; break;
	lazy(FOR, 4);
	lazy(WHILE, 2);
	lazy(IF, 2);
    }
#undef lazy

    for (int i = 0; i < count; ++i) {
	print_recurse(n->params + i, depth + 1, out);
    }
}

void
ast_print(FILE *out)
{
    print_recurse(&ctx.tree, 0, out);
}
