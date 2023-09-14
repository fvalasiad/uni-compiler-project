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
    tac->tcount = 21;		       /* 0-20 reserved by MIX */

    tac->loops_capacity = TAC_DEFAULT_CAPACITY;
    tac->loops_size = 0;
    tac->loops = malloc(tac->loops_capacity * sizeof (loop));
    if (!tac->loops) {
	fprintf(stderr, "error : %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }
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
tac_loops_push(three_address_code *tac, char is_for_loop)
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

    if (is_for_loop) {
	l->label_step = tac->label++;
	l->step_params = malloc(tac->vars_size * sizeof (int));
	if (l->step_params == NULL) {
	    fprintf(stderr, "error : %s\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}

	for (int i = 0; i < tac->vars_size; ++i) {
	    l->step_params[i] = tac->tcount++;
	}
    } else {
	l->label_step = -1;
    }

    l->t = malloc(tac->vars_size * sizeof (int));
    if (l->t == NULL) {
	fprintf(stderr, "error : %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }

    for (int i = 0; i < tac->vars_size; ++i) {
	l->t[i] = tac->tcount++;
    }

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

	    tac->vars_size = n->size;
	    tac->vars = malloc(tac->vars_size * sizeof (int));
	    for (int i = 0; i < tac->vars_size; ++i) {
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
	    statement *s = tac_next(tac);

	    s->type = SASSIGN;
	    s->tx = tac->tcount++;
	    s->ty = arg;

	    tac->vars[n->params->i] = s->tx;
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
	    const loop *l = tac_loops_top(tac);

	    statement *s = tac_next(tac);

	    s->type = SJ;
	    s->size = tac->vars_size;
	    s->t = malloc(2 * s->size * sizeof (int));
	    if (s->t == NULL) {
		fprintf(stderr, "error : %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	    }

	    s->tx = l->label_end;

	    for (int i = 0; i < s->size; ++i) {
		s->t[2 * i] = tac->vars[i];
		s->t[2 * i + 1] = l->t[i];
	    }

	    break;
	}
	case ECONTINUE:{
	    const loop *l = tac_loops_top(tac);

	    statement *s = tac_next(tac);

	    s->type = SJ;
	    s->size = tac->vars_size;
	    s->t = malloc(2 * s->size * sizeof (int));
	    if (s->t == NULL) {
		fprintf(stderr, "error : %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	    }

	    if (l->label_step > 0) {
		s->tx = l->label_step;

		for (int i = 0; i < s->size; ++i) {
		    s->t[2 * i] = tac->vars[i];
		    s->t[2 * i + 1] = l->step_params[i];
		}

	    } else {
		s->tx = l->label_start;

		for (int i = 0; i < s->size; ++i) {
		    s->t[2 * i] = tac->vars[i];
		    s->t[2 * i + 1] = l->t[i];
		}
	    }
	    break;
	}
	case EFOR:{
	    loop *l = tac_loops_push(tac, 1 /* is_for_loop = true */ );

	    /* Initialize counter */
	    recurse(n->params, tac);

	    statement *s = tac_next(tac);

	    s->type = SJ;
	    s->tx = l->label_start;

	    s->size = tac->vars_size;
	    s->t = malloc(2 * s->size * sizeof (int));
	    if (s->t == NULL) {
		fprintf(stderr, "error : %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	    }

	    for (int i = 0; i < s->size; ++i) {
		s->t[2 * i] = tac->vars[i];
		tac->vars[i] = l->t[i];
		s->t[2 * i + 1] = tac->vars[i];
	    }

	    /* Add a NOOP with a label at the start */
	    s = tac_next(tac);

	    s->type = SLABEL;
	    s->tx = l->label_start;

	    s->size = tac->vars_size;
	    s->t = l->t;

	    /* Expression, jump to end if zero */
	    int arg = recurse(n->params + 1, tac);

	    s = tac_next(tac);
	    s->type = SJZ;
	    s->tx = arg;
	    s->ty = l->label_end;

	    s->size = 0;

	    /* Next, follows the block */
	    recurse(n->params + 3, tac);

	    /* Finally, add the step and jump to start */
	    s = tac_next(tac);
	    s->type = SJ;
	    s->tx = l->label_step;
	    s->size = tac->vars_size;
	    s->t = malloc(2 * s->size * sizeof (int));
	    if (s->t == NULL) {
		fprintf(stderr, "error : %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	    }

	    for (int i = 0; i < s->size; ++i) {
		s->t[2 * i] = tac->vars[i];
		tac->vars[i] = l->step_params[i];
		s->t[2 * i + 1] = tac->vars[i];
	    }

	    s = tac_next(tac);	       /* Mark the step to handle continue
				          statements. */
	    s->type = SLABEL;
	    s->tx = l->label_step;
	    s->size = tac->vars_size;
	    s->t = l->step_params;

	    recurse(n->params + 2, tac);

	    s = tac_next(tac);
	    s->type = SJ;
	    s->tx = l->label_start;

	    s->size = tac->vars_size;
	    s->t = malloc(2 * s->size * sizeof (int));
	    if (s->t == NULL) {
		fprintf(stderr, "error : %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	    }

	    for (int i = 0; i < s->size; ++i) {
		s->t[2 * i] = tac->vars[i];
		s->t[2 * i + 1] = l->t[i];
	    }

	    /* Mark the end of the loop */
	    s = tac_next(tac);
	    s->type = SLABEL;
	    s->tx = l->label_end;

	    s->size = 0;

	    for (int i = 0; i < tac->vars_size; ++i) {
		tac->vars[i] = l->t[i];
	    }

	    tac_loops_pop(tac);
	    break;
	}
	case EWHILE:{
	    loop *l = tac_loops_push(tac, 0);

	    statement *s = tac_next(tac);

	    s->type = SJ;
	    s->tx = l->label_start;

	    s->size = tac->vars_size;
	    s->t = malloc(2 * s->size * sizeof (int));
	    if (s->t == NULL) {
		fprintf(stderr, "error : %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	    }

	    for (int i = 0; i < s->size; ++i) {
		s->t[2 * i] = tac->vars[i];
		tac->vars[i] = l->t[i];
		s->t[2 * i + 1] = tac->vars[i];
	    }

	    /* Add a NOOP to label the start */
	    s = tac_next(tac);

	    s->type = SLABEL;
	    s->tx = l->label_start;

	    s->size = tac->vars_size;
	    s->t = l->t;

	    /* Expression, jump to end if zero */
	    int arg = recurse(n->params, tac);

	    s = tac_next(tac);

	    s->type = SJZ;
	    s->tx = arg;
	    s->ty = l->label_end;

	    s->size = 0;

	    /* Block */
	    recurse(n->params + 1, tac);

	    /* Jump to start */
	    s = tac_next(tac);

	    s->type = SJ;
	    s->tx = l->label_start;

	    s->size = tac->vars_size;
	    s->t = malloc(2 * s->size * sizeof (int));
	    if (s->t == NULL) {
		fprintf(stderr, "error : %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	    }

	    for (int i = 0; i < s->size; ++i) {
		s->t[2 * i] = tac->vars[i];
		s->t[2 * i + 1] = l->t[i];
	    }

	    /* Mark the end */
	    s = tac_next(tac);

	    s->type = SLABEL;
	    s->tx = l->label_end;

	    s->size = 0;

	    for (int i = 0; i < tac->vars_size; ++i) {
		tac->vars[i] = l->t[i];
	    }

	    tac_loops_pop(tac);
	    break;
	}
	case EIF:{
	    if (n->size == 2) {	       /* if case */
		int end_label = tac->label++;
		int arg = recurse(n->params, tac);	/* cond */

		/* If zero jump to end */
		statement *s = tac_next(tac);

		s->type = SJZ;
		s->tx = arg;
		s->ty = end_label;

		s->size = tac->vars_size;
		s->t = malloc(2 * s->size * sizeof (int));
		if (s->t == NULL) {
		    fprintf(stderr, "error : %s\n", strerror(errno));
		    exit(EXIT_FAILURE);
		}

		for (int i = 0; i < s->size; ++i) {
		    s->t[2 * i] = tac->vars[i];
		    tac->vars[i] = tac->tcount++;
		    s->t[2 * i + 1] = tac->vars[i];
		}

		statement *temp = s;

		recurse(n->params + 1, tac);

		s = tac_next(tac);

		s->type = SJ;
		s->tx = end_label;

		s->size = tac->vars_size;
		s->t = malloc(2 * s->size * sizeof (int));
		if (s->t == NULL) {
		    fprintf(stderr, "error : %s\n", strerror(errno));
		    exit(EXIT_FAILURE);
		}

		for (int i = 0; i < s->size; ++i) {
		    s->t[2 * i] = tac->vars[i];
		    s->t[2 * i + 1] = temp->t[2 * i + 1];
		}

		s = tac_next(tac);

		s->type = SLABEL;
		s->tx = end_label;

		s->size = tac->vars_size;
		s->t = malloc(s->size * sizeof (int));
		if (s->t == NULL) {
		    fprintf(stderr, "error : %s\n", strerror(errno));
		    exit(EXIT_FAILURE);
		}

		for (int i = 0; i < s->size; ++i) {
		    s->t[i] = temp->t[2 * i + 1];
		    tac->vars[i] = s->t[i];
		}
	    } else {		       /* if-else case */
		int else_label = tac->label++;
		int arg = recurse(n->params, tac);	/* cond */

		/* If zero jump to else */
		statement *s = tac_next(tac);

		s->type = SJZ;
		s->tx = arg;
		s->ty = else_label;

		s->size = 0;

		/* the "if" block */
		recurse(n->params + 1, tac);

		int end_label = tac->label++;

		s = tac_next(tac);

		s->type = SJ;
		s->tx = end_label;

		s->size = tac->vars_size;
		s->t = malloc(2 * s->size * sizeof (int));
		if (s->t == NULL) {
		    fprintf(stderr, "error : %s\n", strerror(errno));
		    exit(EXIT_FAILURE);
		}

		for (int i = 0; i < s->size; ++i) {
		    s->t[2 * i] = tac->vars[i];
		    tac->vars[i] = tac->tcount++;
		    s->t[2 * i + 1] = tac->vars[i];
		}

		statement *temp = s;

		s = tac_next(tac);

		s->type = SLABEL;
		s->tx = else_label;

		s->size = 0;

		/* The "else" block */
		recurse(n->params + 2, tac);

		s = tac_next(tac);

		s->type = SJ;
		s->tx = end_label;

		s->size = tac->vars_size;
		s->t = malloc(2 * s->size * sizeof (int));
		if (s->t == NULL) {
		    fprintf(stderr, "error : %s\n", strerror(errno));
		    exit(EXIT_FAILURE);
		}

		for (int i = 0; i < s->size; ++i) {
		    s->t[2 * i] = tac->vars[i];
		    s->t[2 * i + 1] = temp->t[2 * i + 1];
		}

		s = tac_next(tac);

		s->type = SLABEL;
		s->tx = end_label;

		s->size = tac->vars_size;
		s->t = malloc(s->size * sizeof (int));
		if (s->t == NULL) {
		    fprintf(stderr, "error : %s\n", strerror(errno));
		    exit(EXIT_FAILURE);
		}

		for (int i = 0; i < s->size; ++i) {
		    s->t[i] = temp->t[2 * i + 1];
		    tac->vars[i] = s->t[i];
		}
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
tac_right_shift(three_address_code *tac, int pos, int count)
{
    statement *dest;
    statement *src = tac->statements;

    if (tac->size + count > tac->capacity) {
	while (tac->size + count > tac->capacity) {
	    tac->capacity *= 2;
	}

	dest = malloc(tac->capacity * sizeof (statement));

	for (int i = 0; i < pos; ++i) {
	    memcpy(dest + i, src + i, sizeof (statement));
	}
    } else {
	dest = src;
    }

    for (int i = tac->size - 1; i >= pos; --i) {
	memcpy(dest + i + count, src + i, sizeof (statement));
    }

    if (dest != src) {
	free(src);
	tac->statements = dest;
    }

    tac->size += count;
}

void
tac_deSSA(three_address_code *tac)
{
    int i = 0;

    while (i < tac->size) {
	statement *s = tac->statements + i;

	switch (s->type) {
	    case SJZ:
	    case SJ:{
		if (s->size == 0) {
		    ++i;
		    break;
		}

		int count = s->size;

		tac_right_shift(tac, i, count);
		s = tac->statements + i + count;	/* Might have
							   reallocated */

		for (int j = 0; j < s->size; ++j) {
		    tac->statements[i].type = SASSIGN;

		    tac->statements[i].tx = s->t[2 * j + 1];
		    tac->statements[i].ty = s->t[2 * j];
		    ++i;
		}

		++i;
		break;
	    }
	    default:
		++i;
	}
    }
}

void
tac_print(three_address_code *tac, FILE *out)
{
    statement *s;

#define ONE(instr, s) case S##instr: fprintf(out, "%s t%d\n", #instr, s.tx); break
#define TWO(instr, s) case S##instr: fprintf(out, "%s t%d, t%d\n", #instr, s.tx, s.ty); break
#define THREE(instr, s) case S##instr: fprintf(out, "%s t%d, t%d, t%d\n", #instr, s.tx, s.ty, s.tz); break

    for (int i = 0; i < tac->size; ++i) {
	switch (tac->statements[i].type) {
	    case SMOV:
		fprintf(out, "MOV t%d, %d\n", tac->statements[i].tx,
			tac->statements[i].ty);
		break;
		TWO(ASSIGN, tac->statements[i]);
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
		s = tac->statements + i;
		fprintf(out, "J l%d(", s->tx);
		for (int j = 0; j < s->size - 1; ++j) {
		    fprintf(out, "t%d, ", s->t[2 * j]);
		}
		if (s->size) {
		    fprintf(out, "t%d)\n", s->t[2 * (s->size - 1)]);
		} else {
		    fprintf(out, ")\n");
		}
		break;
	    case SLABEL:
		s = tac->statements + i;
		fprintf(out, "LABEL l%d(", s->tx);
		for (int j = 0; j < s->size - 1; ++j) {
		    fprintf(out, "t%d, ", s->t[j]);
		}
		if (s->size) {
		    fprintf(out, "t%d)\n", s->t[s->size - 1]);
		} else {
		    fprintf(out, ")\n");
		}
		break;
	    case SJZ:
		s = tac->statements + i;
		fprintf(out, "JZ t%d, l%d(", s->tx, s->ty);
		for (int j = 0; j < s->size - 1; ++j) {
		    fprintf(out, "t%d, ", s->t[2 * j]);
		}
		if (s->size) {
		    fprintf(out, "t%d)\n", s->t[2 * (s->size - 1)]);
		} else {
		    fprintf(out, ")\n");
		}
		break;
	}
    }
#undef ONE
#undef TWO
#undef THREE
}
