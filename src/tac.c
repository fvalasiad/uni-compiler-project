#include "types.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

extern context ctx;

#define TAC_DEFAULT_CAPACITY 64

static void
tac_new(three_address_code *tac)
{
    tac->capacity = TAC_DEFAULT_CAPACITY;
    tac->size = 0;
    tac->statements = malloc(tac->capacity * sizeof (statement));
    if (!tac->statements) {
	fprintf(stderr, "error : %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }

    tac->label = 0;
    tac->tcount = 0;
}

static statement *
tac_next(three_address_code *tac)
{
    if (tac->size == tac->capacity) {
	tac->capacity *= 2;
	tac->statements =
		realloc(tac->statements, tac->capacity * sizeof (statement));
	if (!tac->statements) {
	    fprintf(stderr, "error : %s\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}
    }

    return tac->statements + tac->size++;
}

static loop *
tac_loops_push(three_address_code *tac)
{
    if (tac->loops_size == tac->loops_capacity) {
	tac->loops_capacity *= 2;
	tac->loops = realloc(tac->loops, tac->loops_capacity * sizeof (loop));
	if (!tac->loops) {
	    fprintf(stderr, "error : %s\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}
    }

    loop *l = tac->loops + tac->loops_size++;

    l->label_start = tac->label++;
    l->label_end = tac->label++;

    return l;
}

static void
tac_loops_pop(three_address_code *tac)
{
    --tac->loops_size;
}

static const loop *
tac_loops_top(const three_address_code *tac)
{
    return tac->loops + (tac->loops_size - 1);
}

static int
recurse(node *n, three_address_code *tac)
{
    statement *s;

    switch (n->type) {
	case EDECL:{
	    s = tac_next(tac);
	    s->type = SMOV;
	    s->tx = tac->tcount++;
	    s->ty = 0;

	    tac->vars = malloc(n->size * sizeof (int));
	    for (int i = 0; i < n->size; ++i) {
		tac->vars[i] = s->tx;
	    }
	    break;
	}
	case ELITERAL:{
	    s = tac_next(tac);
	    s->type = SMOV;
	    s->tx = tac->tcount++;
	    s->ty = n->i;
	    return s->tx;
	}
	case ENOT:{
	    int arg = recurse(n->params, tac);

	    s = tac_next(tac);
	    s->type = SNOT;
	    s->tx = tac->tcount++;
	    s->ty = arg;

	    return s->tx;
	}
	case EUMINUS:{
	    int arg = recurse(n->params, tac);
	    statement *s = tac_next(tac);

	    s->type = SUMINUS;
	    s->tx = tac->tcount++;
	    s->ty = arg;

	    return s->tx;
	}

/* I am extremely lazy */
#define BINOP(OP) case E##OP: do { \
    	    int arg2 = recurse(n->params + 1, tac); \
    	    int arg1 = recurse(n->params, tac); \
	    s = tac_next(tac); \
	    s->type = S##OP; \
	    s->tx = tac->tcount++; \
	    s->ty = arg1; \
	    s->tz = arg2; \
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
	    int arg = recurse(n->params + 1, tac);

	    tac->vars[n->params->i] = arg;
	    break;
	}
	case EID:{
	    return tac->vars[n->i];
	}
	case EPRINT:{
	    int arg = recurse(n->params, tac);
	    statement *s = tac_next(tac);

	    s->type = SPRINT;
	    s->tx = arg;
	    break;
	}
	case ENOOP:{
	    break;
	}
	case EBREAK:{
	    statement *s = tac_next(tac);

	    s->type = SJ;
	    s->tx = tac_loops_top(tac)->label_end;
	    break;
	}
	case ECONTINUE:{
	    statement *s = tac_next(tac);

	    s->type = SJ;
	    s->tx = tac_loops_top(tac)->label_start;
	    break;
	}
	case EFOR:{
	    loop *l = tac_loops_push(tac);

	    /* Initialize counter */
	    recurse(n->params, tac);

	    /* Add a NOOP with a label at the start */
	    statement *s = tac_next(tac);

	    s->type = SLABEL;
	    s->tx = l->label_start;

	    /* Expression, jump to end if zero */
	    int arg = recurse(n->params + 1, tac);

	    s = tac_next(tac);
	    s->type = SJZ;
	    s->tx = arg;
	    s->ty = l->label_end;

	    /* Next, follows the block */
	    recurse(n->params + 2, tac);

	    /* Finally, add the step and jump to start */
	    recurse(n->params + 3, tac);

	    s = tac_next(tac);
	    s->type = SJ;
	    s->tx = l->label_start;

	    /* Mark the end of the loop */
	    s = tac_next(tac);
	    s->type = SLABEL;
	    s->tx = l->label_end;

	    tac_loops_pop(tac);
	    break;
	}
	case EWHILE:{
	    loop *l = tac_loops_push(tac);

	    /* Add a NOOP to label the start */
	    statement *s = tac_next(tac);

	    s->type = SLABEL;
	    s->tx = l->label_start;

	    /* Expression, jump to end if zero */
	    int arg = recurse(n->params, tac);

	    s = tac_next(tac);

	    s->type = SJZ;
	    s->tx = arg;
	    s->ty = l->label_end;

	    /* Block */
	    recurse(n->params + 1, tac);

	    /* Jump to start */
	    s = tac_next(tac);

	    s->type = SJ;
	    s->tx = l->label_start;

	    /* Mark the end */
	    s = tac_next(tac);

	    s->type = SLABEL;
	    s->tx = l->label_end;

	    tac_loops_pop(tac);
	    break;
	}
	case EIF:{
	    int end_label = tac->label++;
	    int arg = recurse(n->params, tac);

	    /* If zero jump to else block or fi */
	    statement *s = tac_next(tac);

	    s->type = SJZ;
	    s->tx = arg;
	    s->ty = end_label;

	    /* the "if" block */
	    recurse(n->params + 1, tac);

	    if (n->size == 3) {	       /* Do we have an "else" block? */
		/* If so, jump to the very end! */
		int very_end_label = tac->label++;

		s = tac_next(tac);

		s->type = SJ;
		s->tx = very_end_label;

		/* Here resides the end_label */
		s = tac_next(tac);

		s->type = SLABEL;
		s->tx = end_label;

		/* The "else" block */
		recurse(n->params + 2, tac);

		/* Here resides the very end label */
		s = tac_next(tac);

		s->type = SLABEL;
		s->tx = very_end_label;
	    } else {		       /* Or maybe not? */
		/* Then here resides the end label */

		s = tac_next(tac);

		s->type = SLABEL;
		s->tx = end_label;
	    }
	    break;
	}
	case ECOMMA:{
	    for (int i = 0; i < n->size; ++i) {
		recurse(n->params + i, tac);
	    }
	    break;
	}

    }

    return -1;
}

void
ast_to_tac(three_address_code *tac)
{
    tac_new(tac);
    recurse(&ctx.tree, tac);
}

void
tac_print(three_address_code *tac, FILE *out)
{
#define ONE(instr, s) case S##instr: fprintf(out, "%s t%d\n", #instr, s.tx); break
#define TWO(instr, s) case S##instr: fprintf(out, "%s t%d, t%d\n", #instr, s.tx, s.ty); break
#define THREE(instr, s) case S##instr: fprintf(out, "%s t%d, t%d, t%d\n", #instr, s.tx, s.ty, s.tz); break

    for (int i = 0; i < tac->size; ++i) {
	switch (tac->statements[i].type) {
	    case SMOV:
		fprintf(out, "MOV t%d, %d\n", tac->statements[i].tx,
			tac->statements[i].ty);
		break;
		TWO(NOT, tac->statements[i]);
		TWO(UMINUS, tac->statements[i]);
		THREE(MOD, tac->statements[i]);
		THREE(DIV, tac->statements[i]);
		THREE(MUL, tac->statements[i]);
		THREE(SUB, tac->statements[i]);
		THREE(PLUS, tac->statements[i]);
		THREE(BIGGER, tac->statements[i]);
		THREE(BIGGEREQ, tac->statements[i]);
		THREE(LESSEQ, tac->statements[i]);
		THREE(LESS, tac->statements[i]);
		THREE(NOTEQ, tac->statements[i]);
		THREE(EQ, tac->statements[i]);
		THREE(AND, tac->statements[i]);
		THREE(OR, tac->statements[i]);
		ONE(PRINT, tac->statements[i]);
	    case SJ:
		fprintf(out, "J l%d\n", tac->statements[i].tx);
		break;
	    case SLABEL:
		fprintf(out, "LABEL l%d\n", tac->statements[i].tx);
		break;
	    case SJZ:
		fprintf(out, "JZ t%d, l%d\n", tac->statements[i].tx,
			tac->statements[i].ty);
		break;
	}
    }
#undef ONE
#undef TWO
#undef THREE
}
