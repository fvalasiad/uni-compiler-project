#include "types.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

extern context ctx;

#define TAC_DEFAULT_CAPACITY 64

static void
tac_new(three_address_code *tac)
{
    tac->program.capacity = TAC_DEFAULT_CAPACITY;
    tac->program.size = 0;
    tac->program.statements =
	    malloc(tac->program.capacity * sizeof (statement));
    if (!tac->program.statements) {
	fprintf(stderr, "error : %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }
}

static statement *
tac_next(three_address_code *tac)
{
    if (tac->program.size == tac->program.capacity) {
	tac->capacity *= 2;
	tac->statements = malloc(tac->capacity * sizeof (statement));
	if (!tac_statements) {
	    fprintf(stderr, "error : %s\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}
    }

    return tac->statements[tac->size++];
}

static int
recurse(node *n, three_address_code *tac)
{
    statement *s;

    switch (n->type) {
	case EDECL:{
	    tac->vars = malloc(n->size * sizeof (int));
	    for (int i = 0; i < n->size; ++i) {
		s = tac_next(tac);
		s->type = SMOV;
		s->tx = i;
		s->ty = 0;

		tac->vars[i] = i;
	    }
	    break;
	}
	case ELITERAL:{
	    s = tac_next(tac);
	    s->type = SMOV;
	    s->tx = tac->size - 1;
	    s->ty = n->i;
	    return s->tx;
	}
	case ENOT:{
	    s = tac_next(tac);
	    s->type = SNOT;
	    s->tx = tac->size - 1;
	    s->ty = recurse(n->params);

	    return s->tx;
	}
	case EUMINUS:{
	    statement *s = tac_next(tac);

	    s->type = SUMINUS;
	    s->tx = tac->size - 1;
	    s->ty = recurse(n->params);

	    return s->tx;
	}

/* I am extremely lazy */
#define BINOP(OP) case E##OP: do { \
	    s = tac_next(tac); \
	    s->type = S##OP; \
	    s->tx = tac->size - 1; \
	    s->ty = recurse(n->params); \
	    s->tz = recurse(n->params + 1); \
	    }while(0); return s->tx
	    BINOP(MOD);
	    BINOP(DIV);
	    BINOP(MUL);
	    BINOP(SUB);
	    BINOP(PLUS);
	    BINOP(BIGGEREQ);
	    BINOP(BIGGER);
	    BINOP(LESSEQ);
	    BINOP(LESS);
	    BINOP(NOTEQ);
	    BINOP(EQ);
	    BINOP(AND);
	    BINOP(OR);
#undef BINOP
	case EASSIGN:{
	    statement *s = tac_next(tac);

	    s->type = SMOV;
	    s->tx = tac->size - 1;
	    s->ty = recurse(n->params + 1);

	    tac->vars[n->params->i] = s->tx;
	    break;
	}
	case EID:{
	    return tac->vars[n->i];
	}
	case EPRINT:{
	    statement *s = tac_next(tac);

	    s->type = SPRINT;
	    s->tx = recurse(n->params);
	    break;
	}
	case ENOOP:{
	    break;
	}
	case EBREAK:{
	    statement *s = tac_next(tac);

	    s->type = SJ;
	}

    }

    return -1;
}

void
ast_to_tac(three_address_code *tac)
{
    recurse(&ctx.tree, tac);
}
