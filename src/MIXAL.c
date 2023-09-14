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

		inst = MIXAL_next(mixal);
		inst->type = ISTA;
		inst->address = s->tx;

		break;
	    }
	    case SNOT:{
		instruction *inst;

		inst = MIXAL_next(mixal);
		inst->type = ILDA;
		inst->address = s->ty;

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

		inst = MIXAL_next(mixal);
		inst->type = ISTA;
		inst->address = s->tx;
		inst->label = tac->label - 1;
		break;
	    }
	    case SUMINUS:{
		instruction *inst;

		inst = MIXAL_next(mixal);
		inst->type = ILDAN;
		inst->address = s->ty;

		inst = MIXAL_next(mixal);
		inst->type = ISTA;
		inst->address = s->tx;
		break;
	    }
	    case SMOD:{
		instruction *inst;

		inst = MIXAL_next(mixal);
		inst->type = IENTA;
		inst->address = 0;

		inst = MIXAL_next(mixal);
		inst->type = ILDX;
		inst->address = s->ty;

		inst = MIXAL_next(mixal);
		inst->type = IDIV;
		inst->address = s->tz;

		inst = MIXAL_next(mixal);
		inst->type = ISTX;
		inst->address = s->tx;
		break;
	    }
	    case SDIV:{
		instruction *inst;

		inst = MIXAL_next(mixal);
		inst->type = IENTA;
		inst->address = 0;

		inst = MIXAL_next(mixal);
		inst->type = ILDX;
		inst->address = s->ty;

		inst = MIXAL_next(mixal);
		inst->type = IDIV;
		inst->address = s->tz;

		inst = MIXAL_next(mixal);
		inst->type = ISTA;
		inst->address = s->tx;
		break;
	    }
	    case SMUL:{
		instruction *inst;

		inst = MIXAL_next(mixal);
		inst->type = ILDA;
		inst->address = s->ty;

		inst = MIXAL_next(mixal);
		inst->type = IMUL;
		inst->address = s->tz;

		inst = MIXAL_next(mixal);
		inst->type = ISTX;
		inst->address = s->tx;
		break;
	    }
	    case SSUB:{
		instruction *inst;

		inst = MIXAL_next(mixal);
		inst->type = ILDA;
		inst->address = s->ty;

		inst = MIXAL_next(mixal);
		inst->type = ISUB;
		inst->address = s->tz;

		inst = MIXAL_next(mixal);
		inst->type = ISTA;
		inst->address = s->tx;
		break;
	    }
	    case SPLUS:{
		instruction *inst;

		inst = MIXAL_next(mixal);
		inst->type = ILDA;
		inst->address = s->ty;

		inst = MIXAL_next(mixal);
		inst->type = IADD;
		inst->address = s->tz;

		inst = MIXAL_next(mixal);
		inst->type = ISTA;
		inst->address = s->tx;
		break;
	    }
#define LOGICOP(OP) \
		instruction *inst; \
		inst = MIXAL_next(mixal); \
		inst->type = ILDA; \
		inst->address = s->ty; \
\
		inst = MIXAL_next(mixal); \
		inst->type = ICMPA; \
		inst->address = s->tz; \
 \
		inst = MIXAL_next(mixal); \
		inst->type = OP; \
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
		inst = MIXAL_next(mixal); \
		inst->type = ISTA; \
		inst->address = s->tx; \
		inst->label = tac->label - 1;
	    case SBIGGER:{
		LOGICOP(IJG);
		break;
	    }
	    case SBIGGEREQ:{
		LOGICOP(IJGE);
		break;
	    }
	    case SLESSEQ:{
		LOGICOP(IJLE);
		break;
	    }
	    case SLESS:{
		LOGICOP(IJL);
		break;
	    }
	    case SNOTEQ:{
		LOGICOP(IJNE);
		break;
	    }
	    case SEQ:{
		LOGICOP(IJE);
		break;
	    }
#undef LOGICOP
	    case SPRINT:{
		instruction *instr;

		instr = MIXAL_next(mixal);
		instr->type = ILDA;
		instr->address = s->tx;

		instr = MIXAL_next(mixal);
		instr->type = ICHAR;

		instr = MIXAL_next(mixal);
		instr->type = ISTA;
		instr->address = 1951;

		instr = MIXAL_next(mixal);
		instr->type = ISTX;
		instr->address = 1952;

		instr = MIXAL_next(mixal);
		instr->type = IOUT;
		instr->address = 1951;
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
		instruction *inst;

		inst = MIXAL_next(mixal);
		inst->type = ILDA;
		inst->address = s->tx;

		inst = MIXAL_next(mixal);

		inst->type = IJAZ;
		inst->address = s->ty;
		break;
	    }
	    case SJNZ:{
		instruction *inst;

		inst = MIXAL_next(mixal);
		inst->type = ILDA;
		inst->address = s->tx;

		inst = MIXAL_next(mixal);

		inst->type = IJANZ;
		inst->address = s->ty;
		break;
	    }
	    case SASSIGN:{
		instruction *inst;

		inst = MIXAL_next(mixal);
		inst->type = ILDA;
		inst->address = s->ty;

		inst = MIXAL_next(mixal);

		inst->type = ISTA;
		inst->address = s->tx;
	    }
	}
    }
}
