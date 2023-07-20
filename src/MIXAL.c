#include "types.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define MIXAL_DEFAULT_CAPACITY 256

static void
MIXAL_init(MIXAL *mixal)
{
    mixal->capacity = MIXAL_DEFAULT_CAPACITY;
    mixal->size = 0;
    mixal->instructions = malloc(mixal->capacity * sizeof (instruction));

    if (!mixal->instructions) {
	fprintf(stderr, "error : %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }

    mixal->label = -1;
}

static instruction *
MIXAL_next(MIXAL *mixal)
{
    if (mixal->size == mixal->capacity) {
	mixal->capacity *= 2;
	mixal->instructions =
		realloc(mixal->instructions,
			mixal->capacity * sizeof (instruction));
	if (!mixal->instructions) {
	    fprintf(stderr, "error : %s\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}
    }

    mixal->instructions[mixal->size].label = mixal->label;
    mixal->label = -1;

    return mixal->instructions + mixal->size++;
}

static void
store_ra_and_load(MIXAL *mixal, int load)
{
    instruction *inst;

    inst = MIXAL_next(mixal);
    inst->type = ISTA;
    inst->address = mixal->ra;

    inst = MIXAL_next(mixal);
    inst->type = ILDA;
    inst->address = load;
}

void
tac_to_MIXAL(three_address_code *tac, MIXAL *mixal)
{
    MIXAL_init(mixal);
    for (int i = 0; i < tac->size; ++i) {
	statement *s = tac->statements + i;

	switch (s->type) {
	    case SMOV:{
		instruction *inst;

		inst = MIXAL_next(mixal);

		if (s->ty < 0) {
		    inst->type = IENNA;
		    inst->address = -s->ty;
		} else {
		    inst->type = IENTA;
		    inst->address = s->ty;
		}

		mixal->ra = s->tx;
		break;
	    }
	    case SNOT:{
		instruction *inst;

		if (mixal->ra != s->ty) {
		    store_ra_and_load(mixal, s->ty);
		}

		inst = MIXAL_next(mixal);
		inst->type = IJAZ;
		inst->address = tac->label++;

		inst = MIXAL_next(mixal);
		inst->type = IENTA;
		inst->address = 0;

		inst = MIXAL_next(mixal);
		inst->type = IJSJ;
		inst->address = tac->label++;

		inst = MIXAL_next(mixal);
		inst->type = IENTA;
		inst->address = 1;
		inst->label = tac->label - 2;

		mixal->label = tac->label - 1;
		mixal->ra = s->tx;
		break;
	    }
	    case SUMINUS:{
		instruction *inst;

		inst = MIXAL_next(mixal);
		inst->type = ISTA;
		inst->address = mixal->ra;

		inst = MIXAL_next(mixal);
		inst->type = ILDAN;
		inst->address = s->ty;

		mixal->ra = s->tx;
		break;
	    }
	    case SMOD:{
		instruction *inst;

		inst = MIXAL_next(mixal);
		inst->type = ISTA;
		inst->address = mixal->ra;

		inst = MIXAL_next(mixal);
		inst->type = ILDX;
		inst->address = s->ty;

		inst = MIXAL_next(mixal);
		inst->type = IDIV;
		inst->address = s->tz;

		inst = MIXAL_next(mixal);
		inst->type = ISTX;
		inst->address = s->tx;

		mixal->ra = -1;
		break;
	    }
	    case SDIV:{
		instruction *inst;

		inst = MIXAL_next(mixal);
		inst->type = ISTA;
		inst->address = mixal->ra;

		inst = MIXAL_next(mixal);
		inst->type = ILDX;
		inst->address = s->ty;

		inst = MIXAL_next(mixal);
		inst->type = IDIV;
		inst->address = s->tz;

		mixal->ra = s->tx;
		break;
	    }
	    case SMUL:{
		instruction *inst;

		if (mixal->ra == s->ty) {
		    inst = MIXAL_next(mixal);
		    inst->type = IMUL;
		    inst->address = s->tz;
		} else if (mixal->ra == s->tz) {
		    inst = MIXAL_next(mixal);
		    inst->type = IMUL;
		    inst->address = s->ty;
		} else {
		    store_ra_and_load(mixal, s->ty);

		    inst = MIXAL_next(mixal);
		    inst->type = IMUL;
		    inst->address = s->tz;
		}

		inst = MIXAL_next(mixal);
		inst->type = ISTX;
		inst->address = s->tx;

		mixal->ra = -1;
		break;
	    }
	    case SSUB:{
		instruction *inst;

		if (mixal->ra == s->ty) {
		    inst = MIXAL_next(mixal);
		    inst->type = ISUB;
		    inst->address = s->tz;
		} else {
		    store_ra_and_load(mixal, s->ty);

		    inst = MIXAL_next(mixal);
		    inst->type = ISUB;
		    inst->address = s->tz;
		}

		mixal->ra = s->tx;
		break;
	    }
	    case SPLUS:{
		instruction *inst;

		if (mixal->ra == s->ty) {
		    inst = MIXAL_next(mixal);
		    inst->type = IADD;
		    inst->address = s->tz;
		} else if (mixal->ra == s->tz) {
		    inst = MIXAL_next(mixal);
		    inst->type = IADD;
		    inst->address = s->ty;
		} else {
		    store_ra_and_load(mixal, s->ty);

		    inst = MIXAL_next(mixal);
		    inst->type = IADD;
		    inst->address = s->tz;
		}

		mixal->ra = s->tx;
		break;
	    }
#define LOGICOP(OP, OP1) \
		instruction *inst; \
		if (mixal->ra != s->ty && mixal->ra != s->tz) { \
		    store_ra_and_load(mixal, s->ty); \
		    mixal->ra = s->ty; \
		} \
 \
		inst = MIXAL_next(mixal); \
		inst->type = ICMPA; \
		inst->address = s->tz; \
 \
		inst = MIXAL_next(mixal); \
		if (mixal->ra == s->ty) { \
		    inst->type = OP; \
		} else if (mixal->ra == s->tz) { \
		    inst->type = OP1; \
		} \
		inst->address = tac->label++; \
 \
		inst = MIXAL_next(mixal); \
		inst->type = IENTA; \
		inst->address = 0; \
 \
		inst = MIXAL_next(mixal); \
		inst->type = IJSJ; \
		inst->address = tac->label++; \
 \
		inst = MIXAL_next(mixal); \
		inst->type = IENTA; \
		inst->address = 1; \
		inst->label = tac->label - 2; \
 \
		mixal->label = tac->label - 1; \
		mixal->ra = s->tx
	    case SBIGGER:{
		LOGICOP(IJG, IJLE);
		break;
	    }
	    case SBIGGEREQ:{
		LOGICOP(IJGE, IJL);
		break;
	    }
	    case SLESSEQ:{
		LOGICOP(IJLE, IJG);
		break;
	    }
	    case SLESS:{
		LOGICOP(IJL, IJGE);
		break;
	    }
	    case SNOTEQ:{
		LOGICOP(IJNE, IJNE);
		break;
	    }
	    case SEQ:{
		LOGICOP(IJE, IJE);
		break;
	    }
#undef LOGICOP
	    case SAND:{
		instruction *instr;

		if (mixal->ra != s->ty) {
		    store_ra_and_load(mixal, s->ty);
		}

		instr = MIXAL_next(mixal);
		instr->type = IJAZ;
		instr->address = tac->label++;

		instr = MIXAL_next(mixal);
		instr->type = ILDA;
		instr->address = s->tz;

		instr = MIXAL_next(mixal);
		instr->type = IJAZ;
		instr->address = tac->label - 1;

		instr = MIXAL_next(mixal);
		instr->type = IENTA;
		instr->address = 1;

		instr = MIXAL_next(mixal);
		instr->type = IJSJ;
		instr->address = tac->label++;

		instr = MIXAL_next(mixal);
		instr->type = IENTA;
		instr->address = 0;
		instr->label = tac->label - 2;

		mixal->label = tac->label - 1;
		mixal->ra = s->tx;
		break;
	    }
	    case SOR:{
		instruction *instr;

		if (mixal->ra != s->ty) {
		    store_ra_and_load(mixal, s->ty);
		}

		instr = MIXAL_next(mixal);
		instr->type = IJANZ;
		instr->address = tac->label++;

		instr = MIXAL_next(mixal);
		instr->type = ILDA;
		instr->address = s->tz;

		instr = MIXAL_next(mixal);
		instr->type = IJAZ;
		instr->address = tac->label++;

		instr = MIXAL_next(mixal);
		instr->type = IENTA;
		instr->address = 1;
		instr->label = tac->label - 2;

		instr = MIXAL_next(mixal);
		instr->type = IJSJ;
		instr->address = tac->label++;

		instr = MIXAL_next(mixal);
		instr->type = IENTA;
		instr->address = 0;
		instr->label = tac->label - 2;

		mixal->label = tac->label - 1;
		mixal->ra = s->tx;
		break;
	    }
	    case SPRINT:{
		if (mixal->ra != s->tx) {
		    store_ra_and_load(mixal, s->tx);
		    mixal->ra = s->tx;
		}

		instruction *instr = MIXAL_next(mixal);

		instr->type = IOUT;
		break;
	    }
	    case SJ:{
		instruction *inst = MIXAL_next(mixal);

		inst->type = IJSJ;
		inst->address = s->tx;
		break;
	    }
	    case SLABEL:{
		mixal->label = s->tx;
		break;
	    }
	    case SJZ:{
		instruction *inst = MIXAL_next(mixal);

		inst->type = IJAZ;
		inst->address = s->ty;
		break;
	    }
	}
    }
}
